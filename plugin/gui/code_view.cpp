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
#include <assert.h>
#include <math.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <set>
#include <vector>
#include "gtkmm/connect.h"
#include "gtkmm/menu.h"
#include "gtkmm/menuitem.h"
#include "gtkmm/tooltips.h"
#include "dharma/environ.h"
#include "dharma/path.h"
#include "dharma/symbol_util.h"
#include "zdk/breakpoint_util.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/symbol.h"
#include "zdk/thread_util.h"
#include "zdk/zero.h"
#include "zdk/zobject_scope.h"
#include "case_insensitive.h"
#include "ensure_font.h"
#include "fixed_font.h"
#include "gui.h"
#include "html_view.h"
#include "lower_fun.h"
#include "memory_req.h"
#include "popup_menu.h"
#include "scope_helpers.h"
#include "slot_macros.h"
#include "states.h"
#include "code_view_common.h"

#define ASM_PAGE_SIZE   256

using namespace std;
using namespace SigC;
using namespace Gtk;


////////////////////////////////////////////////////////////////
bool CodeView::is_different_file(const Symbol* sym) const
{
    if (!sym)
    {
        return symbol_;
    }
    return Text::get_length() == 0
        || file_.is_null()
        || !file_->is_equal2(sym->file());
}


////////////////////////////////////////////////////////////////
CodeView::CodeView
(
    Debugger& debugger,
    MenuBuilder<CodeView>& builder,
    const string& fontName
)
    : Text()
    , foreGnd_("black")
    , backGnd_("white")
    , lineCol_("blue")
    , font_(fontName)
    , debugger_(debugger)
    , menuBuilder_(builder)
    , curLine_(0)
    , lower_(0)
    , upper_(0)
    , type_(VIEW_SOURCE)
    , searchResult_(0)
    , menuContext_(*this)
    , inChangeSource_(false)
    , fallenBack_(false)
{
    set_line_wrap(false);
    set_editable(false);
    set_name("CodeView");

    if (fontName == fixed_font())
    {
        ensure_monospace(font_, *this);
    }
    init();
}


CodeView::~CodeView() throw()
{
    reset_tips();
}


////////////////////////////////////////////////////////////////
///
/// PUBLIC method, clears text AND all internal state
///
void CodeView::clear()
{
    reset();
    symbol_.reset();
    thread_.reset();
    frame_.reset();
    file_.reset();
    search_.erase();
    searchResult_ = 0;
    fallenBack_ = false;
    type_ = VIEW_SOURCE;
}


////////////////////////////////////////////////////////////////
///
/// Internal method: clears the text but keeps a things such as
/// symbol, thread, frame; useful when changing mode from say,
/// source to disassembly.
///
void CodeView::reset()
{
    assert_ui_thread();
    ScopedFreeze<Text> freeze(*this);

    lineMap_.clear();
    addrMap_.clear();
    reverseLineMap_.clear();
    breakpointMap_.clear();
    upper_ = lower_ = curLine_ = 0;

    reset_text();

    rClick_.clear();
    wordAtCursor_.clear();
    reset_tips();
}


////////////////////////////////////////////////////////////////
static inline
bool equiv_type(const CodeView& view, ViewType viewType)
{
    return (viewType == VIEW_DEFAULT)
        || (viewType == view.view_type());
}


////////////////////////////////////////////////////////////////
bool CodeView::update_source
(
    const RefPtr<Symbol>&   sym,
    const RefPtr<Thread>&   thread,
    const RefPtr<Frame>&    frame,
    bool                    hilite,
    ViewType                viewType,
    size_t&                 line
)
{
    bool result = true; // got code?

    // handle DISASM and MIXED view on the else branch
    // even if the file hasn't changed, because the
    // address range may differ; disasm_async handles it

    if ((view_type() == VIEW_SOURCE)
        && equiv_type(*this, viewType)
        && !is_different_file(sym.get())
       )
    {
        set_symbol(sym, thread, frame);
    }
    else
    {
        if (change_source_file(sym, thread, frame, hilite, viewType))
        {
            return false;
        }
        if (line && (view_type() == VIEW_SOURCE))
        {
            result = read_source_file(sym->file(), sym->addr());

            // if the source file has been truncated / edited
            // since the last time the debuggee was built, then
            // it is possible for the line number not to be
            // found; in this event, fall back to disassembly
            if (line_count() < line)
            {
                result = false;
            }
        }
        else
        {
            result = false;
        }
        if (result)
        {
            set_symbol(sym, thread, frame);
        }
        else
        {
            bool fallback = fallenBack_;

            if ((view_type() == VIEW_SOURCE) || (viewType == VIEW_SOURCE))
            {
                fallback = true; // fallback to disassembly
                viewType = VIEW_MIXED;
            }
            if (fallback)
            {
                // assume: disassembler that uses its own
                // memory buffers is slower, turn off source
                if (Disassembler* da = interface_cast<Disassembler*>(&debugger_))
                {
                    if (da->uses_own_buffer())
                    {
                        viewType = VIEW_DISASSEMBLED;
                    }
                }
            }
            set_view_type(viewType);
            set_symbol(sym, thread, frame);

            // set_view_type resets fallenBack_, need to restore:
            fallenBack_ = fallback;
            line = 0;
            result = disasm_async(*CHKPTR(thread), *CHKPTR(sym), hilite);
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
void CodeView::set_symbol
(
    const RefPtr<Symbol>& sym,
    const RefPtr<Thread>& thread,
    const RefPtr<Frame>& frame
)
{
    if (!symbol_
        || symbol_->addr() != sym->addr()
        || frame_ != frame
        || thread_ != thread)
    {
        thread_ = thread;
        symbol_ = sym;
        frame_ = frame;

        // emit a signal to let the main window redisplay
        // information related to the symbol in view:

        symbol_changed(thread, sym, frame.get());
    }
}


////////////////////////////////////////////////////////////////
bool CodeView::bring_in_view
(
    const RefPtr<Symbol>& sym,
    const RefPtr<Thread>& thread,
    const RefPtr<Frame>& frame,
    bool hilite,
    ViewType viewType
)
{
    CHKPTR(thread);
    CHKPTR(sym);

    if (file().is_null())
    {
        set_frame(frame);
        set_thread(thread.get());
        return false;
    }
    if (viewType != VIEW_DEFAULT)
    {
        set_view_type(viewType);
    }
    else if (this->thread() == thread)
    {
        set_frame(frame);
        return false;
    }
    if (disasm_fallen_back() &&
        (viewType == VIEW_SOURCE || viewType == VIEW_DEFAULT))
    {
        viewType = VIEW_SOURCE;
    }
    set_elements_in_view(sym, thread, frame, hilite, viewType);
    return true;
}


////////////////////////////////////////////////////////////////
void CodeView::set_elements_in_view
(
    const RefPtr<Symbol>&   sym,
    const RefPtr<Thread>&   thread,
    const RefPtr<Frame>&    frame,
    bool                    hilite,
    ViewType                viewType
)
{
    assert_ui_thread();

    if ((symbol_ == sym)    &&
        (thread_ == thread) &&
        (frame_ == frame)   &&
        (sym.is_null() || sym->line() == curLine_) &&
        equiv_type(*this, viewType)
       )
    {
        scroll_to_line(curLine_);
        return;
    }
    if (thread)
    {
        size_t line = 0;

        if (view_type() == VIEW_SOURCE)
        {
            if (sym)
            {
                line = sym->line();
            }
        }

        if (update_source(sym, thread, frame, hilite, viewType, line))
        {
            frame_ = frame;
            thread_ = thread;

            if (!line)
            {
                if (sym)
                {
                    line = line_by_addr(sym->addr());
                }
            }
            if (hilite)
            {
                goto_line(line);
            }
            else
            {
                scroll_to_line(line);
            }
        }
        else
        {
            dbgout(0) << __func__ << ": update_source=false\n";
            thread_ = thread;
        }
    }
    else
    {
        dbgout(0) << __func__ << ": null thread" << endl;
    }
}


////////////////////////////////////////////////////////////////
void CodeView::set_view_type(ViewType type, bool clearAll)
{
    if (!equiv_type(*this, type))
    {
        if (clearAll)
        {
            RefPtr<Thread> thread(thread_);
            clear();
            assert(!symbol_);
            thread_.swap(thread);
        }
        else
        {
            reset();
        }
        type_ = type;
    }
}


////////////////////////////////////////////////////////////////
bool CodeView::have_disassembly() const
{
    return (lower_ || upper_);
}


////////////////////////////////////////////////////////////////
addr_t CodeView::next_addr(addr_t addr) const
{
    if (addr == 0)
    {
        if (!symbol_)
        {
            return 0;
        }
        else
        {
            addr = symbol_->addr();
        }
    }
    LineMap::const_iterator i = addrMap_.upper_bound(addr);

    // todo: disassemble some more if at end if
    // we have reached the end of the map?
    if (i != addrMap_.end())
    {
        assert(i->first > addr);
        return i->first;
    }
    return 0;
}



////////////////////////////////////////////////////////////////
bool
CodeView::disasm_async(Thread& thread, Symbol& sym, bool hilite)
{
    bool haveCode = true;
    ZObjectScope scope;
    const SymbolTable* symtab = sym.table(&scope);
    if (!symtab)
    {
        dbgout(0) << __func__ << ": no symbol table\n";
        return false;
    }
    const addr_t addr = sym.addr();
    assert(sym.addr() <= addr);

    // is this symbol in another file than the file in view?
    if (is_different_file(&sym))
    {
        reset();
        // reset post-condition:
        assert(lower_ == 0 && upper_ == 0);

        set_file(sym.file());
        file_changed(this);
    }
    //
    // if the address of the current symbol is out of the
    // current range of disassembled addresses, disassemble
    // more code -- otherwise it's a no-op
    //
    if (addr < lower_ || addr >= upper_)
    {
        assert(file_);

        haveCode = false;
        reset();
        set_point(0);

        // try disassembling from the begining of the current
        // translation unit (source file)
        RefPtr<Symbol> start;
        if (addr_t low = lower_fun(debugger_, sym))
        {
            start = symtab->lookup_symbol(low);
        }
        if (start.is_null())
        {
            dbgout(0) << __func__ << ": lower_fun failed\n";
        }
        else if (start->addr() > addr)
        {
            dbgout(0) << __func__ << ": lower_fun out of range\n";

            start.reset();
        }

        addr_t begin = start.is_null() ? symtab->addr() : start->addr();

        // attempt to do the whole module
        addr_t end = symtab->upper();

        if (RefPtr<TranslationUnit> unit =
            debugger_.lookup_unit_by_addr(thread.process(), addr))
        {
            end = unit->upper_addr();
        }

        if (end <= addr)
        {
    #ifdef DEBUG
            clog << __func__ << ": upper table bound lower than addr:\n";
            clog << symtab->filename() << ": end=";
            clog << hex << end << dec << endl;
            clog << RefPtr<Symbol>(&sym) << endl;
    #endif
            end = addr + ASM_PAGE_SIZE;
        }

        // address should be in range
        assert(begin <= addr);
        assert(end >= addr);

        if (begin == addr)
        {
            begin = addr - ASM_PAGE_SIZE / 16;
            start.reset();
        }
        // disassemble only a portion, if range is too large
        // todo: are these arbitrary values okay?
        if (end - begin > 10 * ASM_PAGE_SIZE)
        {
            if (begin + ASM_PAGE_SIZE / 2 < addr)
            {
                begin = addr - ASM_PAGE_SIZE / 2;
                start.reset();
            }
            end = addr + ASM_PAGE_SIZE / 2;
        }
        if (get_length() == 0)
        {
            assert(curLine_ == 0);
            assert(lineMap_.empty());
        }
        if (start.is_null())
        {
            ZObjectScope scope;
            if (SymbolTable* table = sym.table(&scope))
            {
                if (sym.offset() < ASM_PAGE_SIZE)
                {
                    begin = sym.addr() - sym.offset();
                    end = begin + ASM_PAGE_SIZE;
                }
                start = table->lookup_symbol(begin);
            }
        }
        if (start.is_null())
        {
            dbgout(0) << __func__ << ": fallback to " << sym
                      << " [" << hex << addr << "-" << end << ")\n";
            start = &sym;
            begin = addr; // symbol->addr();
            end = addr + ASM_PAGE_SIZE;
        }
        assert(end >= begin);
        assert(start);
        assert(start->addr() == begin);

        RefPtr<MemoryRequest> memReq =
            new MemoryRequest(start, end - begin, type_, hilite, file_);

        if (Disassembler* da = interface_cast<Disassembler*>(&debugger_))
        {
            if (da->uses_own_buffer())
            {
                // bypass asynchronous reading
                on_read_done(memReq);
                return true;
            }
          #if DEBUG
            else
            {
                clog << "asynchronous disassembly\n";
            }
          #endif
        }
        if (symtab->addr())
        {
            haveCode = true;
            read_memory_async(memReq);
        }
        else
        {
            assert(symtab->filename());
            dbgout(0) << __func__ << " unmapped: " << symtab->filename() << endl;
        }
    }
    return haveCode;
}



/**
 * Done reading async buffer proceed to disassembly
 */
void CodeView::on_read_done(RefPtr<MemoryRequest> mem)
{
    RefPtr<Thread> thr = thread();
    if (!get_chars().empty())
    {
    #ifdef DEBUG
        clog << "*** Warning: already got source ";
        if (mem->file)
        {
            clog << "for " << mem->file->c_str();
        }
        if (file_)
        {
             clog << ": " << file_->c_str();
        }
        clog << endl;
    #endif
        //clear();
        reset();
    }
    if (!thr)
    {
    #if DEBUG
        //throw logic_error(__func__ + string(": null thread"));
    #endif
        return;
    }
    assert(mem);

    // has the view type changed between request and response?
    if (view_type() != mem->viewType)
    {
        assert(false);
        return;
    }

    ostringstream msg;
    msg << "Disassembling " << mem->requestedSize << " bytes at " << mem->begin;
    status_message(msg.str());
    dbgout(0) << "Actual buffer size=" << mem->bytes.size() << endl;

    set_asm_tabstops();

    { // do not update the text widget until we leave this scope
        ScopedFreeze<Text> freeze(*this);

        debugger_.disassemble(thr.get(),
                              mem->begin.get(),
                              mem->requestedSize,
                              (view_type() == VIEW_MIXED),
                              &mem->bytes[0],
                              this);
        if (lower_ == 0)
        {
            lower_ = mem->begin->addr();
        }
        if (upper_ == 0)
        {
            upper_ = mem->begin->addr() + mem->bytes.size();
        }
        //
        // enumerate breakpoints to mark them in a different color
        //
        if (BreakPointManager* mgr = interface_cast<BreakPointManager*>(&debugger_))
        {
            EnumBreakPoints observ(*this);
            mgr->enum_breakpoints(&observ);
        }
    }

    const size_t line = line_by_addr(CHKPTR(symbol_)->addr());
    if (mem->hilite)
    {
        goto_line(line);
    }
    else
    {
        scroll_to_line(line);
    }
    symbol_changed(thr, symbol_, NULL);
}



/**
 * Callback for Debugger::disassemble
 */
bool CodeView::notify(addr_t addr, const char* line, size_t len)
{
    assert(line);
    static const size_t LEN = sizeof(addr_t) * 2;

    const Gdk_Color* foreGnd = &foreGnd_;

    if ((len < LEN + 1) || (line[LEN] != ':'))
    {
        foreGnd = &lineCol_;
    }
    const size_t n = line_count() + 1;

    if (addr)
    {
        if (addr < lower_ || lower_ == 0)
        {
            lower_ = addr;
        }
        if (addr > upper_)
        {
            upper_ = addr;
        }

        addrMap_.insert(make_pair(addr, n));
    }
    const size_t pnt = get_length();

    check_before_insert_asm_line(pnt);

    lineMap_[n] = pnt;
    reverseLineMap_[pnt] = n;

    Context context = get_context();
    context.set_font(font_);
    context.set_foreground(*foreGnd);

    string s(line);
    insert(context, ensure_new_line(s));

    return true; // continue disassembling
}


////////////////////////////////////////////////////////////////
void CodeView::goto_line(size_t line)
{
    scroll_to_line(line);
    hilite_line(curLine_, false);
    hilite_line(line, true);
    curLine_ = line;
}


////////////////////////////////////////////////////////////////
void CodeView::hilite_line(size_t line, bool hilite)
{
    if (line)
    {
        BreakPointState state = query_breakpoint_at_line(line);
        hilite_line(line, hilite, state);
    }
}


////////////////////////////////////////////////////////////////
void CodeView::mark_line
(
    size_t lineNum,
    const Gdk_Color& foreGnd,
    const Gdk_Color& backGnd
)
{
    LineMap::const_iterator i = lineMap_.find(lineNum);

    if (i == lineMap_.end())
    {
        dbgout(0) << hex << lineNum << dec << " line not in map\n.";

        if (view_type() != VIEW_SOURCE)
        {
            i = lineMap_.lower_bound(lineNum);
            if (i != lineMap_.begin()) --i;
        }
    }
    if (i != lineMap_.end())
    {
        const size_t begin = i->second;
        const size_t end   = begin + line_number_width();
        const string text = get_chars(begin, end);

        set_point(begin);

        ScopedFreeze<Text> freeze(*this);
        forward_delete(line_number_width());
        insert(font_,
            const_cast<Gdk_Color&>(foreGnd),
            const_cast<Gdk_Color&>(backGnd), text, -1);
    }
}


////////////////////////////////////////////////////////////////
void CodeView::scroll_to_position(size_t pos)
{
    LineMap::const_iterator i = reverseLineMap_.lower_bound(pos);
    if (i == reverseLineMap_.end() || i->first != pos)
    {
        if (i == reverseLineMap_.begin())
        {
            return;
        }
        --i;
    }
    scroll_to_line(i->second);
}


////////////////////////////////////////////////////////////////
BEGIN_SLOT(CodeView::scroll_to_line,(size_t line))
{
    if (line == 0)
    {
        return;
    }
    if (line > line_count())
    {
        clog << __func__ << ": line " << line;
        clog << " is out of range (max=" << line_count() << ")\n";
        return;
    }
    assert(lineMap_.find(line) != lineMap_.end());

    assert(line > 0);
    --line;
    scroll_to_line_internal(line);
}
END_SLOT()


////////////////////////////////////////////////////////////////
BreakPointState CodeView::query_breakpoint_at_line(size_t line) const
{
    BreakPointState result = B_NONE;

    RefPtr<Thread> thr = thread();
    if (!thr || !thr->process())
    {
        return result;
    }
    // Get the breakpoint manager for the current process:
    const pid_t pid = CHKPTR(thr->process())->pid();

    // ... then ask the debugger for the manager object,
    BreakPointManager* mgr = debugger_.breakpoint_manager(pid);

    BreakPointMap::const_iterator i = breakpointMap_.find(line);

    if ((mgr != NULL) && (i != breakpointMap_.end()))
    {
        const BreakPointList& list = i->second;

        BreakPointList::const_iterator j = list.begin();
        for (; j != list.end(); ++j)
        {
            // filter out breakpoints that don't apply to this process
            if (!mgr->enum_breakpoints(NULL, NULL, *j))
            {
                continue;
            }

            if (has_enabled_user_breakpoint_actions(debugger_, *j))
            {
                result = B_ENABLED;
                break;
            }
            else if (has_disabled_user_breakpoint_actions(debugger_, *j))
            {
                result = B_DISABLED;
            }
        }
    }
    return result;
}


CodeView::LineMap::const_iterator
CodeView::offs_to_line(size_t pos) const
{
    LineMap::const_iterator i = reverseLineMap_.lower_bound(pos);
    if (i == reverseLineMap_.end() || i->first != pos)
    {
        if (i == reverseLineMap_.begin())
        {
            return reverseLineMap_.end();
        }
        --i;
    }
    return i;
}


/**
 * Collect a list of addresses that correspond to the current
 * position in the assembly listing.
 * Addresses are collected into the RightClickInfo object.
 */
void CodeView::get_asm_addr(size_t pos)
{
    assert(have_disassembly());
    assert(rClick_.addrs().empty());
    dbgout(0) << __func__ << ": pos=" << pos << endl;

    LineMap::const_iterator i = offs_to_line(pos);
    if (i == reverseLineMap_.end())
    {
        return;
    }
    const size_t lineNumber = i->second;
    dbgout(0) << __func__ << ": line=" << lineNumber << endl;

    AddrMap::const_iterator j = addrMap_.begin();
    for (; j != addrMap_.end(); ++j)
    {
        if (j->second != lineNumber)
        {
            continue;
        }
        // collect code addresses that match the line num
        rClick_.push_back(j->first);
        dbgout(0) << __func__ << ": " << (void*)j->first << endl;

        // collect all lines that match the address
        AddrMap::const_iterator k = addrMap_.find(j->first);
        for (; (k != addrMap_.end()) && (k->first == j->first); ++k)
        {
            rClick_.add_line(k->second);
        }
    }
}




namespace
{
    /**
     * Callback functor passed to enum_addresses_by_line.
     */
    class ZDK_LOCAL LineToAddr
        : public Debugger::AddrEvents
        , public EnumCallback<addr_t>
    {
        RightClickInfo& rclick_;
        set<addr_t> unique_;

        RefPtr<SymbolTable> table_;
        bool mapped_;

    public:
        LineToAddr(addr_t programCounter, RightClickInfo& rclick)
            : rclick_(rclick)
            , table_(NULL)
            , mapped_(true)
        {
            rclick.set_program_count(programCounter);
        }

        void set_mapped(bool mapped) { mapped_ = mapped; }
        void set_table(SymbolTable* table) { table_ = table; }

        void notify(SymbolTable* table, addr_t addr)
        {
            Temporary<bool> saveFlag(mapped_);
            Temporary<RefPtr<SymbolTable> > setTable(table_, table);

            if (table)
            {
                table->ensure_loaded();
                if (table->addr() == 0)
                {
                    mapped_ = false;
                }
            }
            else
            {
                mapped_ = false;
            }

            notify(addr);
        }

        void notify(addr_t addr)
        {
            if (unique_.insert(addr).second)
            {
                //dbgout(0) << __func__ << ": " << (void*)addr << endl;

                if (mapped_)
                {
                    rclick_.push_back(addr);
                }
                else if (table_)
                {
                    if (RefPtr<Symbol> sym = table_->lookup_symbol(addr))
                    {
                        rclick_.add_deferred_symbol(sym);
                    }
                    else
                    {
                        ostringstream errmsg;
                        errmsg << "Symbol not found: "  << (void*)addr;
                        errmsg << " in " << table_->filename();

                        throw logic_error(errmsg.str());
                    }
                }
                else assert(false);
            }
        }

        size_t addr_count() const { return unique_.size(); }
    };
}


////////////////////////////////////////////////////////////////
void CodeView::get_src_addr(size_t pos)
{
    assert(!have_disassembly());

    if (RefPtr<Thread> thr = thread())
    {
        get_src_addr(pos, thr);
    }
#ifdef DEBUG
    else
    {
        clog << "WARNING: " << __func__ << ": null thread!\n";
    }
#endif
}


////////////////////////////////////////////////////////////////
void
CodeView::get_src_addr(size_t pos, const RefPtr<Thread>& thr)
{
    assert(file_);
    assert(thr);
    if (!file_ || !thr)
    {
        return;
    }
    assert(rClick_.addrs().empty());
    dbgout(0) << __func__ << ": pos=" << pos << endl;
    addr_t programCount = 0;

    if (Frame* frame = thread_current_frame(thr.get()))
    {
        programCount = frame->program_count();
    }
    LineMap::const_iterator i = offs_to_line(pos);

    if (i != reverseLineMap_.end())
    {
        const size_t lineNum = i->second;
        rClick_.add_line(lineNum);

        dbgout(0) << __func__ << ": line=" << lineNum
                  << " sym="  << symbol_   << endl;

        LineToAddr addrCB(programCount, rClick_);

        if (symbol_)
        {
            ZObjectScope scope;
            if (SymbolTable* tbl = symbol_->table(&scope))
            {
                addrCB.set_table(tbl);
                tbl->ensure_loaded();
                if (tbl->addr() == 0)
                {
                    addrCB.set_mapped(false);
                }
                const size_t matchCount =
                    tbl->enum_addresses_by_line(file_.get(), lineNum, &addrCB);
                dbgout(0) << __func__ << ": " << matchCount << " match"
                          << (matchCount > 1 ? "es." : ".") << endl;
            }
        }
        else
        {
            dbgout(0) << __func__ << ": using Debugger::line_to_addr" << endl;
            debugger_.line_to_addr(file_.get(), lineNum, &addrCB, thr.get());
        }
    }
    else
    {
        dbgout(0) << __func__ << ": pos not found " << pos << "\n";
    }
}



////////////////////////////////////////////////////////////////
void CodeView::get_addrs_at_offs(size_t pos)
{
    assert_main_thread();
    if (symbol_)
    {
        rClick_.set_program_count(symbol_->addr());
    }
    if (have_disassembly())
    {
        get_asm_addr(pos);
    }
    else
    {
        get_src_addr(pos);
    }
}


////////////////////////////////////////////////////////////////
bool CodeView::context_menu_add_eval(Gtk::Menu& menu)
{
    DebugSymbolList debugSyms;

    addr_t addr = rClick_.nearest_addr();
    if (!addr && symbol_)
    {
        addr = symbol_->addr();
    }

    // these are the delimiters used by get_word_at_position:
    // .,;+-/*%|&~^=<>()[]{}!\"' \t
    // there is no point in calling query_symbols when the token
    // starts with a special character
    const string& token = wordAtCursor_;

    if (!token.empty() && !isdigit(token[0]) &&
        string(".,;/%|=<>()[]{}").find(token[0]) == string::npos)
    {
        query_symbols(token, addr, &debugSyms, false);
    }

    MenuItem* item = 0;

    if (debugSyms.empty())
    {
     /* slow, leave it out for now

        // no matching symbols found, try types
        RefPtr<DataType> type = query_type(token, addr);

        if (type.is_null())
        {
            return false;
        }

        item = manage(new MenuItem("What is " + token + "?"));
        item->activate.connect(bind(what_is.slot(), type));
      */
    }
    else
    {
        item = manage(Gtk_image_menu_item(&Stock::ZOOM_IN, "Evaluate"));

        // selecting the menu item brings up a view
        // of the variable(s) matching the name

        Gtk_CONNECT(item, activate, bind(evaluate.slot(), debugSyms));
    }
    if (item)
    {
        menu.append(*item);
        item->show();
        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////
size_t CodeView::get_line(Symbol& sym) const
{
    if (file_.is_null())
    {
        dbgout(0) << __func__ << ": no file in view.\n";
        return 0;
    }
    if (!sym.file())
    {
        return 0;
    }

    if (is_different_file(&sym))
    {
        dbgout(0) << __func__ << ": " << file_->c_str() << endl;
        dbgout(0) << "breakpoint=" << sym.file() << endl;

        return 0;
    }

    size_t line = sym.line();

    if (have_disassembly())
    {
        const addr_t addr = sym.addr();

        LineMap::const_iterator i = addrMap_.find(addr);
        if (i == addrMap_.end())
        {
            return 0;
        }
        line = i->second;
    }
    return line;
}


////////////////////////////////////////////////////////////////
bool CodeView::on_insert_breakpoint
(
    const RefPtr<Symbol>& sym,
    bool isDeferred
)
{
    assert_ui_thread();
    assert(sym);

    const addr_t addr = sym->addr();
    const size_t line = get_line(*sym);
    bool result = true;

    if (line)
    {
        result = breakpointMap_[line].insert(addr).second;
    #if DEBUG
        print_symbol(clog << __func__ << ' ', addr, sym);
        clog << endl << __func__ << " result=" << result << endl;
    #endif             
        if (curLine_ != line)
        {
            hilite_line(line, false);
        }
        //
        // just in case the line where the breakpoint is being
        // inserted happens to be the currently marked line
        //
        hilite_line(curLine_, true);
    }
    return result;
}


////////////////////////////////////////////////////////////////
void
CodeView::on_remove_breakpoint(const RefPtr<Symbol>& sym,
                               bool isDeferred)
{
    assert_ui_thread();
    assert(sym);

    addr_t addr = sym->addr();
    if (isDeferred)
    {
        ZObjectScope scope;
        SymbolTable* table = CHKPTR(sym->table(&scope));
        addr -= table->adjustment();
    }
    const size_t line = get_line(*sym);

    BreakPointMap::iterator j = breakpointMap_.find(line);
    if (j == breakpointMap_.end())
    {
        dbgout(0) << __func__ << ": line not found: " << line << endl;
    }
    else
    {
        BreakPointList& v = j->second;
        assert(!v.empty());

        BreakPointList::iterator i = v.find(addr);
        if (i == v.end())
        {
            dbgout(0) << __func__ << ": line=" << line
                      << " addr not found " << (void*)addr << endl;
        }
        else
        {
            v.erase(i);
            dbgout(0) << __func__ << ": " << v.size()
                      << " breakpoint(s) left at line "
                      << line << endl;
        }
        if (v.empty())
        {
            breakpointMap_.erase(j);
            hilite_line(line, false);
            hilite_line(curLine_, true);
        }
    }
}


////////////////////////////////////////////////////////////////
void CodeView::breakpoint_state_changed(addr_t addr)
{
    BreakPointMap::iterator i = breakpointMap_.begin();

    for (; i != breakpointMap_.end(); ++i)
    {
        BreakPointList::iterator j = i->second.begin();
        BreakPointList::iterator end = i->second.end();

        for (; j != end; ++j)
        {
            if (addr != *j)
            {
                continue;
            }
            const size_t line = i->first;

            BreakPointState s = query_breakpoint_at_line(line);

            hilite_line(line, line == curLine_, s);
        }
    }
}


////////////////////////////////////////////////////////////////
void CodeView::show_next_stat(addr_t addr)
{
    show_next_line(addr, false);
}


////////////////////////////////////////////////////////////////
void CodeView::open_folder()
{
    string fileMan = env::get("ZERO_FILE_MANAGER", "nautilus");
    string folder = sys::dirname(file()->c_str());

    open_url_in_browser(fileMan, folder);
}


////////////////////////////////////////////////////////////////
void CodeView::show_cur_stat(addr_t addr)
{
    show_next_line(addr, true);
}


////////////////////////////////////////////////////////////////
void CodeView::show_func_start (addr_t addr)
{
    if (thread_)
    {
        RefPtr<Symbol> sym;
        SymbolMap* symbols = thread_->symbols();
        //
        // step 1: find the current code symbol
        //
        if (addr)
        {
            if (!symbols)
            {
                return;
            }
            sym = symbols->lookup_symbol(addr);
        }
        else
        {
            sym = thread_current_function(thread_.get());
        }
        //
        // step 2: if the symbol has an offset, it means
        // that it is a synthesised one; look for the actual symbol
        //
        if (sym && sym->offset() && symbols)
        {
            addr = sym->addr() - sym->offset();
            sym = symbols->lookup_symbol(addr);

            // todo: handle the is_different_file case
            if (sym && !is_different_file(sym.get()))
            {
                goto_line(sym->line());
            }
        }
    }
}


////////////////////////////////////////////////////////////////
BEGIN_SLOT(CodeView::search,(const string& what))
{
    // cache search string, for search_impl
    search_ = what;

    if (!what.empty())
    {
        debugger_.properties()->set_string("search", what.c_str());

        if (searchResult_ == 0)
        {
            searchResult_ = get_position();
        }
        search_impl(searchResult_);
    }
}
END_SLOT()


////////////////////////////////////////////////////////////////
BEGIN_SLOT(CodeView::search_again,())
{
    if (!search_.empty())
    {
        size_t fromPosition = searchResult_;

        if (debugger_.properties()->get_word("search_forward", 1))
        {
            fromPosition += search_.length();
        }
        search_impl(fromPosition);
    }
}
END_SLOT()


////////////////////////////////////////////////////////////////
void CodeView::search_impl(size_t fromPosition)
{
    assert(!search_.empty());

    if (debugger_.properties()->get_word("search_forward", true))
    {
        search_forward(fromPosition);
    }
    else
    {
        search_backward(fromPosition);
    }
}


////////////////////////////////////////////////////////////////
void CodeView::search_backward(size_t fromPosition)
{
    static const size_t bufsize = 1024;

    size_t tmp = 0;
    for (size_t pos = fromPosition; pos > 0; pos = tmp)
    {
        if (pos > bufsize)
        {
            tmp = pos - bufsize;
        }
        else
        {
            tmp = 0;
        }
        string buf = get_chars(tmp, pos + search_.length() - 1);
        assert(!buf.empty());

        if (search_buf(tmp, buf, false))
        {
            return;
        }
    }
    if (string_not_found(search_))
    {
        search_forward(0);
    }
}


////////////////////////////////////////////////////////////////
void CodeView::search_forward(size_t fromPosition)
{
    static const size_t bufsize = 1024;
    const size_t length = get_length();

    for (size_t tmp = length;;)
    {
        for (size_t pos = fromPosition; pos != length; pos = tmp)
        {
            if (length - pos > bufsize)
            {
                tmp = pos + bufsize;
            }
            else
            {
                tmp = length;
            }
            size_t n = pos;
            if (n > search_.length() - 1)
            {
                n -= search_.length() - 1;
            }

            string buf = get_chars(n, tmp);
            if (search_buf(n, buf))
            {
                return;
            }
        }
        if (!string_not_found(search_))
        {
            break;
        }
        fromPosition = 0;
    }
}


////////////////////////////////////////////////////////////////
bool CodeView::search_buf(size_t pos, const string& buf, bool fwd)
{
    assert(!search_.empty());
    bool found = false;

    size_t n = string::npos;
    word_t caseInsensitive = debugger_.properties()->get_word("case_ins", 0);
    if (caseInsensitive)
    {
        CaseInsensitiveString& key = (CaseInsensitiveString&)search_;
        n = fwd ? ((CaseInsensitiveString&)buf).find(key)
                : ((CaseInsensitiveString&)buf).rfind(key);
    }
    else
    {
        n = fwd ? buf.find(search_) : buf.rfind(search_);
    }
    if (n != string::npos)
    {
        found = true;
        pos += n;

        // highlight the search result
        select_region(pos, pos + search_.length());

        // ensure the result is visible
        scroll_to_position(pos);

        // save the result
        searchResult_ = pos;
    }
    return found;
}


////////////////////////////////////////////////////////////////
string CodeView::selection() const
{
    string sel;

    if (has_selection())
    {
        sel = get_chars(get_selection_start_pos(),
                        get_selection_end_pos());
    }
    return sel;
}


////////////////////////////////////////////////////////////////
unsigned int CodeView::line_number_width() const
{
    return have_disassembly() ? (2 * sizeof(addr_t)) : 8;
}


////////////////////////////////////////////////////////////////
void CodeView::ensure_rclick_line()
{
    const size_t pos = rClick_.position();

    if (rClick_.lines().empty())
    {
        LineMap::const_iterator i = reverseLineMap_.find(pos);
        if (i != reverseLineMap_.end())
        {
            rClick_.add_line(i->second);
        }
    }
}


////////////////////////////////////////////////////////////////
bool CodeView::context_menu_add_breakpoint_items(Menu& menu)
{
    ensure_rclick_line();
    bool result = false;

    RightClickInfo::LineList lines = rClick_.lines();

    for (RightClickInfo::LineList::const_iterator i = lines.begin();
         i != lines.end();
         ++i)
    {
        result |= context_menu_add_breakpoint_items(menu, *i);
    }
    dbgout(0) << __func__ << "=" << result << endl;
    return result;
}


////////////////////////////////////////////////////////////////
bool
CodeView::context_menu_add_breakpoint_items(Menu& menu,
                                            size_t lineNum)
{
    bool result = false;

    set<addr_t> disable_list;
    set<addr_t> enable_list;

    // have any breakpoints at this line?
    BreakPointMap::const_iterator k(breakpointMap_.find(lineNum));

    if (k != breakpointMap_.end())
    {
        result = true;

        const BreakPointList& addrList = k->second;
        BreakPointList::const_iterator v = addrList.begin();
        for (; v != addrList.end(); ++v)
        {
            const addr_t addr = *v;

            // todo: thread safety
            if (has_disabled_user_breakpoint_actions(debugger_, addr))
            {
                enable_list.insert(addr);
            }
            if (has_enabled_user_breakpoint_actions(debugger_, addr))
            {
                disable_list.insert(addr);
            }
        }
        menu_add_breakpoint_item(menu, "Remove Breakpoint",
                     lineNum, addrList, delete_breakpoint,
                     &Gtk::Stock::CANCEL);
        menu_add_breakpoint_item(menu, "Disable Breakpoint",
                     lineNum, disable_list, disable_breakpoint);
        menu_add_breakpoint_item(menu, "Enable Breakpoint",
                     lineNum, enable_list, enable_breakpoint,
                     &Gtk::Stock::APPLY);
    }

    dbgout(0) << __func__ << "=" << result << endl;
    return result;
}


////////////////////////////////////////////////////////////////
template<typename Iterator>
inline void connect(Iterator first,
                    Iterator last,
                    MenuItem* item,
                    size_t line,
                    SigC::Signal2<void, addr_t, size_t>& sig)
{
    for (; first != last; ++first)
    {
        Gtk_CONNECT(item, activate, bind(sig.slot(), *first, line));
    }
    item->show();
}


////////////////////////////////////////////////////////////////
void CodeView::menu_add_breakpoint_item
(
    Menu& menu,
    const string& text,
    size_t line,
    const set<addr_t>& addrList,
    SigC::Signal2<void, addr_t, size_t>& sig,
    const BuiltinStockID* stockID
 )
{
    if (addrList.empty())
    {
        return;
    }
    MenuItem* item = manage(Gtk_image_menu_item(stockID, text));

    if (addrList.size() == 1)
    {
        Gtk_CONNECT(item, activate, bind(sig.slot(), *addrList.begin(), line));
    }
    else if (addrList.size() <= 10)
    {
        // the menu applies to several addresses, create submenu
        Menu* submenu = manage(new Menu);
        item->set_submenu(*submenu);

        // add "All" entry
        MenuItem* subitem = manage(new MenuItem("All"));
        submenu->append(*subitem);
        connect(addrList.begin(), addrList.end(), subitem, line, sig);

        // add individual menu entries for each address in the set
        submenu->items().push_back(Menu_Helpers::SeparatorElem());

        set<addr_t>::const_iterator i = addrList.begin();
        for (i = addrList.begin(); i != addrList.end(); ++i)
        {
            ostringstream text;

            text << "0x" << hex << setfill('0');
            text << setw(2 * sizeof(addr_t)) << *i;

            MenuItem* subitem = manage(new MenuItem(text.str()));
            Gtk_CONNECT(subitem, activate, bind(sig.slot(), *i, line));

            subitem->show();
            submenu->append(*subitem);
        }
    }
    else
    {
        connect(addrList.begin(), addrList.end(), item, line, sig);
    }
    item->show();
    menu.append(*item);
}


/**
 * Work around strange bug with submenu in contextual menus,
 * where items do not always emit signal_activate.
 */
static bool on_button_press(GdkEventButton* event, MenuItem* item)
{
    if (event && event->button == 1)
    {
        item->activate();
    }
    return true;
}


////////////////////////////////////////////////////////////////
void CodeView::add_step_over_item
(
    Gtk::Menu& menu,
    const string& text,
    SigC::Signal1<void, RefPtr<Symbol> >* sig,
    const BuiltinStockID* stock
)
{
    MenuItem* item = manage(Gtk_image_menu_item(stock, text));
    Gtk_CONNECT_1(item, activate, this, &CodeView::step_over, sig);
    menu.append(*item);
    Gtk_CONNECT(item, button_press_event,
                Gtk_BIND(Gtk_PTR_FUN(on_button_press), item));
    item->show();
}



////////////////////////////////////////////////////////////////
///
/// Override Gtk::TextView method
///
void CodeView::on_populate_popup(Menu* menu)
{
    customize_popup_menu(*CHKPTR(menu));
    menuBuilder_.populate(this, *menu, menuContext_);

#if !defined(GTKMM_2)
    wordAtCursor_ = get_word_at_position(rClick_.position());
#endif
    if (symbol_ && !wordAtCursor_.empty())
    {
        context_menu_add_eval(*menu);
        wordAtCursor_.clear();
    }
    MenuItem* item = manage(Gtk_image_menu_item(&Stock::REDO, "Always Step Over..."));
    menu->append(*item);
    Menu* submenu = manage(new Menu);
    add_step_over_item(*submenu, "Function At Cursor", &step_over_func);
    add_step_over_item(*submenu, "Functions In This File", &step_over_file);
    add_step_over_item(*submenu, "Files In This Directory", &step_over_dir);
    submenu->items().push_back(Menu_Helpers::SeparatorElem());

    add_step_over_item(*submenu, "Manage...", &step_over_manage, &Stock::PREFERENCES);

    // add_step_over_item(*submenu, "Clear All", &step_over_reset);

    item->set_submenu(*submenu);
    item->show();
}


////////////////////////////////////////////////////////////////
void CodeView::set_thread(Thread* thread)
{
    if (thread)
    {
        thread_ = thread;
    }
    else
    {
        thread_.reset();
    }
}


////////////////////////////////////////////////////////////////
bool CodeView::change_source_file(const RefPtr<Symbol>& sym,
                                  const RefPtr<Thread>& thread,
                                  const RefPtr<Frame>& frame,
                                  bool hilite,
                                  ViewType viewType)
{
    bool result = false;
    if (!inChangeSource_)
    {
        Temporary<bool> setFlag(inChangeSource_, true);
        result = bring_in_view(sym, thread, frame, hilite, viewType);
    }
    return result;
}


////////////////////////////////////////////////////////////////
void CodeView::apply_font(const Gdk_Font& font)
{
    font_ = font;
    redraw();
}


////////////////////////////////////////////////////////////////
void CodeView::redraw()
{
    RefPtr<Thread> thread(thread_);
    ViewType type = view_type();

    if (symbol_)
    {
        RefPtr<Symbol> symbol(symbol_);
        RefPtr<Frame> frame(frame_);

        clear();
        type_ = type; // restore view type
        set_elements_in_view(symbol, thread, frame, true, type);
    }
    else if (file_)
    {
        RefPtr<SharedString> file(file_);
        clear();
        type_ = type; // restore view type
        read_source_file(file);
    }
}



////////////////////////////////////////////////////////////////
void CodeView::run_to_cursor()
{
    if (const addr_t addr = rClick_.nearest_addr())
    {
        static RightClickInfo tmp;
        tmp.clear();
        tmp.push_back(addr);

        set_breakpoints(&tmp, false);
    }
    else
    {
        insert_temp_breakpoint();
    }
}



////////////////////////////////////////////////////////////////
void CodeView::insert_breakpoint()
{
    set_breakpoints.emit(&rClick_, true);
}


////////////////////////////////////////////////////////////////
void CodeView::insert_temp_breakpoint()
{
    set_breakpoints.emit(&rClick_, false);
}



////////////////////////////////////////////////////////////////
void CodeView::set_program_counter()
{
    set_program_count.emit(rClick_.nearest_addr());
}


////////////////////////////////////////////////////////////////
void CodeView::show_next_statement()
{
    show_next_line(rClick_.nearest_addr(), false);
}


////////////////////////////////////////////////////////////////
void CodeView::show_function_start()
{
    show_func_start(rClick_.nearest_addr());
}


////////////////////////////////////////////////////////////////
void CodeView::step_over(SigC::Signal1<void, RefPtr<Symbol> >* sig)
{
    if (thread_)
    {
        if (RefPtr<SymbolMap> symbols = thread_->symbols())
        {
            const addr_t addr = rClick_.nearest_addr();
            RefPtr<Symbol> sym = symbols->lookup_symbol(addr);
            sig->emit(sym);
        }
        else
        {
            sig->emit(NULL);
        }
    }
#if DEBUG
    else
    {
        clog << __func__ << ": no thread\n";
    }
#endif
}


////////////////////////////////////////////////////////////////
bool CodeView::getline(istream& is, char* buf, size_t buflen)
{
    size_t i = 0;

    for (char c = 0; (i != buflen) && is.get(c); )
    {
        bool eol = false;
        if ((c == 0) || (c == '\n'))
        {
            eol = true;
        }
        else if (c == '\r')
        {
            eol = true;
            if (is.get(c)) // expect CR-LF sequence
            {
                if (c != '\n')
                {
                    is.putback(c);
                }
            }
        }
        if (eol)
        {
            buf[i] = 0;
            break;
        }
        buf[i++] = c;
    }
    if (buflen)
    {
        buf[buflen - 1] = 0;
    }
    return true;
}


////////////////////////////////////////////////////////////////
size_t CodeView::line_by_addr(addr_t addr) const
{
    AddrMap::const_iterator i = addrMap_.find(addr);
    return i == addrMap_.end() ? 0 : i->second;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
