//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <string>
#include <boost/utility.hpp>
#include "app_slots.h"
#include "auto_condition.h"
#include "generic/lock.h"
#include "gtkmm/flags.h"
#include "gtkmm/widget.h"
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "engine/query_symbols.h"
#include "engine/query_type.h"
#include "zdk/debug_symbol_list.h"
#include "zdk/check_ptr.h"
#include "zdk/data_filter.h"
#include "zdk/data_type.h"
#include "zdk/zero.h"
#include "zdk/zobject_scope.h"
#include "gui.h"
#include "message_box.h"
#include "popup_list.h" // for auto_complete
#include "text_entry.h" // for auto_complete
#include "slot_macros.h"


using namespace std;
using namespace boost;


namespace
{
    /**
     * Saves stack traces to output stream.
     */
    class ZDK_LOCAL StackTraceSaver
        : public EnumCallback<Process*>
        , public EnumCallback<Thread*>
    {
        ostream& out_;

    public:
        explicit StackTraceSaver(ostream& out) : out_(out) { }

        void notify(Process* proc)
        {
            if (proc)
            {
                out_ << "===== Process " << proc->pid() << " =====\n";
                proc->enum_threads(this);
            }
        }

        void notify(Thread* thread)
        {
            if (thread)
            {
                out_ << "----- Thread lwpid=" << thread->lwpid();
                out_ << " id=" << thread->thread_id() << " -----\n";

                StackTrace* trace = thread->stack_trace();
                for (size_t i = 0; i != trace->size(); ++i)
                {
                    out_ << "#" << i << " ";
                    RefPtr<Symbol> sym = trace->frame(i)->function();
                    if (sym)
                    {
                        out_ << sym;
                    }
                    out_ << endl;
                }
            }
        }
    };
} // namespace


static LookupScope right_click_scope()
{
    static LookupScope lookupScope = LOOKUP_LOCAL;

    if (lookupScope == LOOKUP_LOCAL)
    {
        string scope = env::get_string("ZERO_RIGHT_CLICK_SCOPE", "unit");

        if (scope == "all")
        {
            lookupScope = LOOKUP_ALL;
        }
        else if (scope == "module")
        {
            lookupScope = LOOKUP_MODULE;
        }
        else
        {
            lookupScope = LOOKUP_UNIT;
        }
    }
    return lookupScope;
}


AppSlots::AppSlots(Debugger& dbg) : ThreadMarshaller(dbg)
{
}


void
AppSlots::read_symbol(RefPtr<DebugSymbol> sym, DebugSymbolEvents* events)
{
    if (is_ui_thread())
    {
        call_on_main_thread(&AppSlots::read_symbol, sym, events);
    }
    else
    {
        MainThreadScope scope(mainThreadMutex_, mainThreadReady_);

        if (sym)
        {
            sym->read(events);
        }
    }
}


void AppSlots::save_stack(ostream* outp)
{
    if (is_ui_thread())
    {
        call_on_main_thread(&AppSlots::save_stack, outp);
    }
    else
    {
        MainThreadScope scope(mainThreadMutex_, mainThreadReady_);

        StackTraceSaver saver(*outp);
        debugger().enum_processes(&saver);
    }
}



BEGIN_SLOT_(bool, AppSlots::on_evaluate,
(
    RefPtr<Thread>  thread,
    string          expr,
    addr_t          addr,
    ExprEvents*     events,
    int             base
))
{
    static bool evalComplete = true;

    if (is_ui_thread())
    {
        call_on_main_thread(&AppSlots::on_evaluate,
                            thread,
                            expr,
                            addr,
                            events,
                            base);
        if (!evalComplete)
        {
            post_response(new ResumeCommand);
            dbgout(0) << __func__ << ": resume command posted" << endl;
        }
    }
    else
    {
        MainThreadScope scope(mainThreadMutex_, mainThreadReady_);

        if (thread)
        {
            try
            {
                evalComplete = debugger().evaluate( expr.c_str(),
                                                    thread.get(),
                                                    addr,
                                                    events,
                                                    base);
            }
            catch (const std::exception& e)
            {
                cerr << __func__ << ": " << e.what() << endl;
            }
        }
    }
    return evalComplete;
}
END_SLOT_(true)


BEGIN_SLOT(AppSlots::on_query_symbols,
(
    RefPtr<Thread>      thread,
    string              name,
    addr_t              addr,
    DebugSymbolList*    syms,
    bool                readValues
))
{
    assert(syms);

    if (is_ui_thread())
    {
        call_on_main_thread(&AppSlots::on_query_symbols,
                            thread, name, addr, syms, readValues);
    }
    else
    {
        MainThreadScope scope(mainThreadMutex_, mainThreadReady_);

        if (thread)
        {
            RefPtr<Symbol> fun;

            if (addr)
            {
                fun = CHKPTR(thread->symbols())->lookup_symbol(addr);
            }
            const LookupScope scope = right_click_scope();
            query_debug_symbols(thread, name.c_str(), fun, *syms, scope);
            if (readValues)
            {
                for (DebugSymbolList::iterator i = syms->begin();
                     i != syms->end();
                     ++i)
                {
                    (*i)->read(NULL);
                }
            }
        }
    }
}
END_SLOT()


BEGIN_SLOT_(RefPtr<DataType>, AppSlots::on_query_type,
(
    RefPtr<Thread> thread,
    string name,
    addr_t addr
))
{
    assert(thread.get()); // pre-condition

    static RefPtr<DataType> type;

    if (is_ui_thread())
    {
        call_on_main_thread(&AppSlots::on_query_type, thread, name, addr);
    }
    else
    {
        MainThreadScope scope(mainThreadMutex_, mainThreadReady_);

        if (thread)
        {
            TypeLookupHelper helper(*thread, name, addr);
            debugger().enum_plugins(&helper);

            type = helper.type();
        }
    }
    return type;
}
END_SLOT_(0)



BEGIN_SLOT(AppSlots::step_over_func, (RefPtr<Symbol> sym))
{
    if (sym)
    {
        if (is_ui_thread())
        {
            CALL_MAIN_THREAD_(command(&AppSlots::step_over_func, this, sym));
        }
        else
        {
            //
            // get the beginning of current function
            //
            RefPtr<Symbol> func = sym;
            if (sym->offset())
            {
                addr_t addr = sym->addr() - sym->offset();
                //ZObjectScope scope;
                // func = sym->table(&scope)->lookup_symbol(addr);
                if (RefPtr<Thread> thread = current_thread())
                {
                    func = CHKPTR(thread->symbols())->lookup_symbol(addr);
                }
            }
            if (func)
            {
                dbgout(0) << __func__ << ": " << *func << endl;
                debugger().add_step_over(func->file(), func->line());
            }
        }
    }
}
END_SLOT()


BEGIN_SLOT(AppSlots::step_over_file,(RefPtr<Symbol> sym))
{
    if (sym)
    {
        if (is_ui_thread())
        {
            CALL_MAIN_THREAD_(command(&AppSlots::step_over_file, this, sym));
        }
        else
        {
            dbgout(0) << __func__ << ": " << *sym << endl;
            debugger().add_step_over(sym->file(), 0);
        }
    }
#if DEBUG
    else
    {
        throw runtime_error(__func__ + string(": null symbol"));
    }
#endif
}
END_SLOT()



BEGIN_SLOT(AppSlots::step_over_dir,(RefPtr<Symbol> sym))
{
    if (sym)
    {
        if (is_ui_thread())
        {
            CALL_MAIN_THREAD_(command(&AppSlots::step_over_dir, this, sym));
        }
        else
        {
            dbgout(0) << __func__ << ": " << *sym << endl;
            debugger().add_step_over(sym->file(), -1);
        }
    }
#if DEBUG
    else
    {
        throw runtime_error(__func__ + string(": null symbol"));
    }
#endif
}
END_SLOT()


BEGIN_SLOT(AppSlots::step_over_reset,(RefPtr<Symbol> sym))
{
    if (is_ui_thread())
    {
        CALL_MAIN_THREAD_(command(&AppSlots::step_over_reset, this, sym));
    }
    else
    {
        dbgout(0) << __func__ << endl;
        debugger().remove_step_over(0, 0);
    }
}
END_SLOT()


BEGIN_SLOT(AppSlots::step_over_delete,(RefPtr<SharedString> path, long line))
{
    assert(path);

    if (is_ui_thread())
    {
        CALL_MAIN_THREAD_(command(&AppSlots::step_over_delete, this, path, line));
    }
    else
    {
        dbgout(0) << __func__ << ": " << path->c_str() << ":" << line << endl;
        debugger().remove_step_over(path.get(), line);
    }
}
END_SLOT()


BEGIN_SLOT_(bool,
AppSlots::on_auto_complete,(TextEntry* textEntry,
                            string prev,
                            bool useUnmappedLibs))
{
    if (RefPtr<Thread> thread = current_thread())
    {
        string text = textEntry->get_text(false);

        if (text.empty())
        {
            if (popupList_)
            {
                popupList_->hide();
            }
        }
        else
        {
            FunctionEnum syms; // collects enumeration results

            SymbolTable::LookupMode mode =
                SymbolTable::LKUP_RANGE | SymbolTable::LKUP_DYNAMIC;

            if (useUnmappedLibs)
            {
                mode = (mode | SymbolTable::LKUP_UNMAPPED);
            }

            thread->symbols()->enum_symbols(text.c_str(), &syms, mode);

            set<string> names;

            for (SymbolEnum::const_iterator i = syms.begin();
                    i != syms.end();
                    ++i)
            {
                RefPtr<SharedString> s = (*i)->demangled_name(false);
                if (s && s->length())
                {
                    names.insert(s->c_str());
                }
            }
            if (!popupList_)
            {
                popupList_.reset(new PopupList);
            }
            if (names.empty())
            {
                popupList_->hide();
            }
            else
            {
                popupList_->set_text_entry(textEntry);
                popupList_->show_all();

                if (names.size() > 1)
                {
                    popupList_->set_items(names);
                }
                else if (text.size() > prev.size())
                {
                    text = *names.begin();
                    popupList_->set_text_and_hide(text);
                }
            }
        }
    }
}
END_SLOT_(true)


////////////////////////////////////////////////////////////////
bool
AppSlots::on_apply_filter(RefPtr<DebugSymbol>* symbol,
                          RefPtr<DebugSymbol>  parent,
                          DebugSymbolEvents*   events)
{
    if (!symbol)
    {
        return false;
    }
    if (is_ui_thread())
    {
        call_on_main_thread(&AppSlots::on_apply_filter, symbol, parent, events);

        if (symbol->is_null())
        {
            return false;
        }
    }
    else
    {
        MainThreadScope scope(mainThreadMutex_, mainThreadReady_);

        if (DataFilter* filter = interface_cast<DataFilter*>(&debugger()))
        {
            if (filter->hide(symbol->get()))
            {
                symbol->reset();
                return false;
            }
            RefPtr<DebugSymbol> sym =
                filter->transform(symbol->get(), parent.get(), events);

            if (sym && (sym != *symbol))
            {
                if (events)
                {
                    if (events->is_expanding(symbol->get()))
                    {
                        //sym->read(events);
                        events->symbol_change(sym.get(), symbol->get());
                    }
                }
                *symbol = sym;
            }
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////
BEGIN_SLOT(AppSlots::update_stack_on_main_thread,())
{
    if (is_ui_thread())
    {
        call_on_main_thread(&AppSlots::update_stack_on_main_thread);
    }
    else
    {
        MainThreadScope scope(mainThreadMutex_, mainThreadReady_);
        update_stack_view();
    }
}
END_SLOT()



////////////////////////////////////////////////////////////////
BEGIN_SLOT(AppSlots::on_menu_delete_breakpoint,
(
    addr_t addr,
    size_t line
))
{
    if (is_ui_thread())
    {
        call_on_main_thread(&AppSlots::on_menu_delete_breakpoint, addr, line);

        // When the remove_user_breakpoint call succeeds,
        // an on_remove_breakpoint() notification will be sent.
        // should not need to update, but does it hurt?
        update_breakpoint_view(addr, line);
    }
    else
    {
        MainThreadScope scope(mainThreadMutex_, mainThreadReady_);

        // clear breakpoint globally: this needs to change when
        // per-thread breakpoints are better supported in the UI
        debugger().remove_user_breakpoint(0, 0, addr);
    }
}
END_SLOT()
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
