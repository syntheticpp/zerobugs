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

#include <iostream>
#include <numeric>
#include <sstream>
#include <boost/bind.hpp>
#include "gtkmm/adjustment.h"
#include "gtkmm/color.h"
#include "gtkmm/connect.h"
#include "gtkmm/ctree.h"
#include "gtkmm/eventbox.h"
#include "gtkmm/flags.h"
#include "gtkmm/frame.h"
#include "gtkmm/label.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/style.h"
#include "gtkmm/tooltips.h"
#include "gtkmm/window.h"
#include "dharma/environ.h"
#include "dharma/pipe.h"
#include "generic/lock.h"
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "zdk/data_type.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "zdk/types.h"
#include "zdk/variant.h"
#include "frame_state.h"
#include "highlight_changes.h"
#include "gui.h"
#include "is_reference.h"
#include "output_dialog.h"
#include "popup_menu.h"
#include "scope_helpers.h"
#include "set_cursor.h"
#include "slot_macros.h"
#include "variables_view.h"

using namespace std;
using namespace SigC;


////////////////////////////////////////////////////////////////
VariablesView::~VariablesView()
{
    clear_data();
}


////////////////////////////////////////////////////////////////
VariablesView::VariablesView(Debugger& /* debugger */)
    : base_(0)
    , ctree_(0)
    , isStaleView_(false)
    , updatePending_(false)
{
    Gtk::Frame* frame = manage(new Gtk::Frame);
    pack_start(*frame, true, true);
    Gtk::ScrolledWindow* sw = manage(new Gtk::ScrolledWindow);
    frame->add(*sw);

    static const char* titles[] =
    {
        "Variable", "Value", "Type", 0
    };

    ctree_ = manage(new Tree(titles));
    sw->add(*ctree_);
    sw->set_policy(Gtk_FLAG(POLICY_NEVER), Gtk_FLAG(POLICY_AUTOMATIC));

    ctree_->set_column_editable(1, true);

    ctree_->column(0).set_width(100);
    ctree_->column(1).set_width(150);

    ctree_->column(0).set_passive();
    ctree_->column(1).set_passive();
    ctree_->column(2).set_passive();

    ctree_->set_line_style(Gtk_FLAG(CTREE_LINES_DOTTED));
#if GTKMM_2
    ctree_->set_line_style(Gtk_FLAG(CTREE_LINES_SOLID));
#endif

    // connect CTree signals to internal slots
    Gtk_CONNECT_0(ctree_, tree_expand, this, &VariablesView::on_expand);
    Gtk_CONNECT_0(ctree_, tree_collapse, this, &VariablesView::on_collapse);
    Gtk_CONNECT_0(ctree_, cell_edit, this, &VariablesView::on_cell_edit);
}


////////////////////////////////////////////////////////////////
bool VariablesView::is_expanding(DebugSymbol* sym) const
{
    bool result = false;

    if (sym)
    {
        Lock<Mutex> lock(mx_);
        const SymKey key(*sym);

        set<SymKey>::iterator i = expand_.find(key);

        if (i != expand_.end())
        {
            result = true;
        }
        else
        {
            KeyMap::const_iterator j = keyMap_.find(key);
            if (j != keyMap_.end())
            {
                i = expand_.find(j->second);
                if (i != expand_.end())
                {
                    result = true;
                }
            }
        }

        dbgout(1) << __func__ << ": " << sym->name()
                  << " (" << key << ")=" << result << endl;
    }
    return result;
}


////////////////////////////////////////////////////////////////
void VariablesView::emit_sym_read(const RefPtr<DebugSymbol>& sym)
{
    // avoid deadlock: unlock the mutex in this scope,
    // since reading the symbol may cause a callback
    // (symbol_change, for e.g.) to come in on the main thread
    Unlock<Mutex> unlock(mx_);

    read_symbol.emit(sym, this);
}


////////////////////////////////////////////////////////////////
bool VariablesView::emit_sym_filt(RefPtr<DebugSymbol>& sym)
{
    Unlock<Mutex> unlock(mx_);
    RefPtr<DebugSymbol> old = sym;

    bool result = filter.emit(&sym, sym->parent(), this);
    symbol_change(sym.get(), old.get());

    return result;
}


////////////////////////////////////////////////////////////////
bool VariablesView::notify(DebugSymbol* symbol)
{
    assert(symbol);

    Lock<Mutex> lock(mx_);
    Temporary<bool> pending(updatePending_, true);

    emit_sym_read(symbol);

    RefPtr<DebugSymbol> sym(symbol);

    if (emit_sym_filt(sym))
    {
        debugSymbols_.push_back(sym);

        if (is_expanding(sym.get()))
        {
            emit_sym_read(sym);
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////
//
// Helpers for symbol_change: if a symbol is replaced by
// another, say for example, due to a custom filter that displays
// vectors as C-style arrays, expanded child objects need to
// stay expanded after the substitution.
//
static inline void
expand_symbol(VariablesView&, RefPtr<DebugSymbol>, DebugSymbol*);


namespace
{
    /**
     * Enumerate the child symbols of a given symbol and
     * map them by name (the assumption being that language rules
     * ensure uniqueness).
     */
    class ZDK_LOCAL DebugSymbolNameMap : public DebugSymbolCallback
    {
        typedef std::map<RefPtr<SharedString>, RefPtr<DebugSymbol> > map_type;

        map_type map_;

    BEGIN_INTERFACE_MAP(DebugSymbolNameMap)
        INTERFACE_ENTRY(DebugSymbolCallback)
    END_INTERFACE_MAP()

    public:
        bool notify(DebugSymbol* sym)
        {
            if (sym)
            {
                map_.insert(make_pair(sym->name(), sym));
            }
            return true;
        }

        RefPtr<DebugSymbol> find(RefPtr<SharedString> name) const
        {
            RefPtr<DebugSymbol> result;

            map_type::const_iterator i = map_.find(name);
            if (i != map_.end())
            {
                result = i->second;
            }
            return result;
        }
    };


    class ZDK_LOCAL DebugSymbolExpander : public DebugSymbolCallback
    {
        DebugSymbolNameMap  nameMap_;
        VariablesView&      view_;

    BEGIN_INTERFACE_MAP(DebugSymbolExpander)
        INTERFACE_ENTRY(DebugSymbolCallback)
    END_INTERFACE_MAP()

    public:
        DebugSymbolExpander(const RefPtr<DebugSymbol>& sym,
                            VariablesView& view)
            : view_(view)
        {
            sym->enum_children(&nameMap_);
        }

        bool notify(DebugSymbol* sym)
        {
            if (sym && view_.is_expanding(sym))
            {
                if (RefPtr<DebugSymbol> peer = nameMap_.find(sym->name()))
                {
                    expand_symbol(view_, peer, sym);
                }
                else
                {
                    dbgout(0) << "not found: " << sym->name() << endl;
                }
            }
            return true;
        }
    };
} // namespace


void
expand_symbol(VariablesView& view, RefPtr<DebugSymbol> sym, DebugSymbol* old)
{
    assert(sym);

    view.expand(*sym);
    sym->read(&view);

    if (old)
    {
        DebugSymbolExpander expander(sym, view);
        old->enum_children(&expander);
    }
}


////////////////////////////////////////////////////////////////
/// This is called when a symbol is replaced by another, likely
/// as result of a user-defined DataFilter
void
VariablesView::symbol_change(DebugSymbol* sym, DebugSymbol* old)
{
    if (sym && old && (sym != old))
    {
        SymKey oldKey(*old);
        SymKey symKey(*sym);

        Lock<Mutex> lock(mx_);

        // is the old symbol expanded?
        set<SymKey>::iterator i = expand_.find(oldKey);
        if (i != expand_.end())
        {
            expand_symbol(*this, sym, old);
            expand_.erase(i);
            expand_.insert(symKey);
        }
        else
        {
            // was a replacement for sym provided before?
            KeyMap::iterator k = keyMap_.find(oldKey);
            if (k != keyMap_.end())
            {
                // is the replacement expanded?
                i = expand_.find(k->second);
                if (i != expand_.end())
                {
                    DebugSymbolMap::iterator j = subst_.find(k->second);
                    if (j != subst_.end())
                    {
                        expand_symbol(*this, sym, j->second.get());
                    }
                }
            }
        }
        keyMap_[oldKey] = symKey;
    }
}


////////////////////////////////////////////////////////////////
void VariablesView::display(bool force)
{
    Lock<Mutex> lock(mx_);

    if (is_visible() && (force || is_stale_view()))
    {
        set_is_stale_view(false);

        set_cursor(*this, Gdk_FLAG(TOP_LEFT_ARROW));

        ScopedFreeze<Gtk::CTree> freeze(*CHKPTR(ctree_));

        // save the current vertical scroll position
        Gtk::Adjustment* adj = ctree_->get_vadjustment();
        const double adjVal = adj ? adj->get_value() : 0;

        ctree_->reset_tooltip();
        Gtk::clear_rows(*ctree_);

        Gtk::CTree::RowList rows = ctree_->rows();
        DebugSymbolList::const_iterator i = debugSymbols_.begin();
        for (; i != debugSymbols_.end(); ++i)
        {
            add_symbol(rows, *i);
        }
        // restore vertical scroll position
        if (adj)
        {
            adj->set_value(adjVal);
        }
        DebugSymbolList(debugSymbols_).swap(keepAlive_);
    }
}


////////////////////////////////////////////////////////////////
void VariablesView::clear_data(bool keepExpand) throw()
{
    Lock<Mutex> lock(mx_);

    clear_symbols();
    values_.clear();
    subst_.clear();

    if (!keepExpand)
    {
        expand_.clear();
        keyMap_.clear();
    }
}


////////////////////////////////////////////////////////////////
void VariablesView::reset(bool keepExpand)
{
    assert(ctree_);
    Gtk::clear_rows(*ctree_);
    clear_data(keepExpand);
}


////////////////////////////////////////////////////////////////
class VariablesView::EnumChildrenHelper : public DebugSymbolCallback
{
public:
    EnumChildrenHelper(VariablesView& view, Gtk::CTree::RowList rows, bool ref)
      : view_(view), rows_(rows), ref_(ref)
    { }

    bool notify(DebugSymbol* sym)
    {
        if (sym)
        {
            assert(sym->ref_count() > 0);
            view_.add_symbol(rows_, sym, ref_);
        }
        return true;
    }

    BEGIN_INTERFACE_MAP(EnumChildSymbol)
        INTERFACE_ENTRY(DebugSymbolCallback)
    END_INTERFACE_MAP()

private:
    VariablesView& view_;
    Gtk::CTree::RowList rows_;
    bool ref_;
};


////////////////////////////////////////////////////////////////
void VariablesView::restore_state(const Frame* f) volatile
{
    if (f)
    {
        RefPtr<FrameState> state =
            interface_cast<FrameState*>(f->get_user_object(".state"));
        restore_state(state, true);
    }
}


////////////////////////////////////////////////////////////////
void VariablesView::restore_state
(
    const RefPtr<FrameState>& state,
    bool restoreSymbols
) volatile
{
    Lock<Mutex> lock(mx_);
    VariablesView* THIS = const_cast<VariablesView*>(this);
    THIS->subst_.clear();

    if (state.is_null())
    {
        THIS->expand_.clear();
        THIS->values_.clear();

        if (restoreSymbols)
        {
            THIS->debugSymbols_.clear();
            dbgout(0) << __func__ << ": symbols cleared" << endl;
        }
    }
    else
    {
        dbgout(0) << __func__ << endl;
        THIS->expand_ = state->expand_;
        THIS->values_ = state->values_;

        if (restoreSymbols)
        {
            THIS->debugSymbols_ = state->symbols_;
            dbgout(0) << __func__ << ": "
                      << THIS->debugSymbols_.size() << " symbol(s)"
                      << endl;
            THIS->set_is_stale_view(true);
        }
    }
}


////////////////////////////////////////////////////////////////
void VariablesView::save_state(FrameState& state) volatile
{
    VariablesView* THIS = const_cast<VariablesView*>(this);
    state.expand_ = THIS->expand_;
    state.values_ = THIS->values_;
    state.symbols_ = THIS->debugSymbols_;
}


////////////////////////////////////////////////////////////////
RefPtr<SharedString>
VariablesView::validate(const RefPtr<DebugSymbol>& symbol)
{
    RefPtr<SharedString> value = symbol->value();

  #if GTKMM_2
    if (value)
    {
        Glib::ustring line = value->c_str();
        if (!line.validate())
        {
            value.reset();
        }
    }
  #endif

    return value;
}




////////////////////////////////////////////////////////////////
void VariablesView::add_symbol(Gtk::CTree::Row row,
                               RefPtr<DebugSymbol> symbol,
                               RefPtr<SharedString> value)
{
    if (!value)
    {
        dbgout(1) << __func__ << ": " << symbol->name() << ":nil" << endl;
    }
    else
    {
        const SymKey key(*symbol);

        // if returned by a function, or the value has changed
        // from the last displayed value, then show it in another
        // color

        DebugValuesMap::const_iterator i = values_.find(key);

        if (i == values_.end())
        {
            if (symbol->is_return_value())
            {
                row.set_foreground(Gdk_Color(highlight_changes_color()));
            }
        }
        else
        {
            // has the value has changed since last displayed?
            DataType* type = CHKPTR(symbol->type());

            if (!value->is_equal2(i->second.get())
             && type->compare(value->c_str(), CHKPTR(i->second)->c_str()) != 0)
            {
                row.set_foreground(Gdk_Color(highlight_changes_color()));
            }
        }
        values_[key] = value;
        subst_[key] = symbol;

        row.set_data(symbol.get());
    }
}



////////////////////////////////////////////////////////////////
Gtk::CTree::Row
VariablesView::add_row(const char* name,
                       const char* value,
                       const char* type)
{
    assert_ui_thread();

    vector<string> items(3);
    if (name)
    {
        items[0] = name;
    }
    if (value)
    {
        items[1] = value;
    }
    if (type)
    {
        items[2] = type;
    }

    if (ctree_)
    {
        ctree_->rows().push_back(Gtk::CTree::Element(items));
        return ctree_->rows().back();
    }
    return Gtk::CTree::Row();
}


////////////////////////////////////////////////////////////////
void VariablesView::add_symbol(Gtk::CTree::RowList rows,
                               RefPtr<DebugSymbol> symbol,
                               bool ref)
{
    assert_ui_thread();

    if (!CHKPTR(symbol)->type())
    {
    #ifdef DEBUG
        clog << "Symbol has NULL type: " << symbol->name() << endl;
    #endif
        return;
    }

    bool isRef = false;
#if 0
    // Do not create an extra node for the reference object,
    // just show the referred variable (child object).
    if (is_ref(symbol.get()))
    {
        assert(!ref);
        isRef = true;
        expand_.insert(SymKey(*symbol));
    }
    else
#endif
    {
        string name = CHKPTR(symbol->name())->c_str();
        if (ref && symbol->parent())
        {
            name = CHKPTR(symbol->parent()->name())->c_str();
        }
     /* if (symbol->is_return_value())
        {
            name += " returned";
        }
      */
        RefPtr<SharedString> value = validate(symbol);

        const char* items[3] =
        {
            name.c_str(),
            value.is_null() ? "" : value->c_str(),
            CHKPTR(symbol->type_name())->c_str(),
        };

        rows.push_back(Gtk::CTree::Element(items));
        Gtk::CTree::Row row = rows.back();
        rows = row.subtree();
        if (is_expanding(symbol.get()))
        {
            row.expand();
        }

        add_symbol(row, symbol, value);
    }
    // add the children of aggregated symbols (such
    // as class instances, arrays, etc.
    EnumChildrenHelper children(*this, rows, isRef);
    symbol->enum_children(&children);
}


////////////////////////////////////////////////////////////////
void VariablesView::on_expand(Gtk::CTree::Row row)
{
    if (DebugSymbol* sym = reinterpret_cast<DebugSymbol*>(row.get_data()))
    {
        const SymKey key(*sym);
        expand_.insert(key);

        assert(is_expanding(sym));

        if (SharedString* value = sym->value())
        {
            values_.insert(make_pair(key, value));
            subst_.insert(make_pair(key, sym));
        }
        set_cursor(*this, Gdk_FLAG(WATCH));

        dbgout(1) << __func__ << ": " << sym->name() << endl;
        symbol_expand(sym);
    }
}


////////////////////////////////////////////////////////////////
void VariablesView::on_collapse(Gtk::CTree::Row row)
{
    if (DebugSymbol* symbol = reinterpret_cast<DebugSymbol*>(row.get_data()))
    {
        const SymKey key(*symbol);
        expand_.erase(key);
    }
}


////////////////////////////////////////////////////////////////
event_result_t
VariablesView::on_button_press_event(GdkEventButton* event)
{
    Gtk::VBox::on_button_press_event(event);

    if ((event != NULL)                   &&
        (event->type == GDK_BUTTON_PRESS) &&
        (event->button == 3))
    {
        Gtk::RowHandle row;
        int col = 0; // not used

        // silence off compiler warnings; event->x and event->y
        // are of the double type, and get_selection_info expects
        // integers
        const int x = static_cast<int>(event->x);
        const int y = static_cast<int>(event->y);

        if (ctree_ && ctree_->get_selection_info(x, y, &row, &col))
        {
            void* data = ctree_->row(row).get_data();

            if (DebugSymbol* sym = reinterpret_cast<DebugSymbol*>(data))
            {
                popup_menu(*event, sym);
            }
           #ifdef DEBUG
            else
            {
                clog << "data=" << data << endl;
            }
           #endif
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////
void VariablesView::popup_menu
(
    GdkEventButton& event,
    RefPtr<DebugSymbol> symbol
)
{
    assert(symbol.get());
    std::auto_ptr<PopupMenu> menu(new PopupMenu);

    Gtk::MenuItem* item = 0;

    const addr_t addr = symbol->addr();
    if (addr)
    {
        item = menu->add_manage_item(new Gtk::MenuItem("View Raw Memory"));
        Gtk_CONNECT(item, activate, Gtk_BIND(show_raw_memory.slot(), addr));
    }

    Gtk::CheckMenuItem* check =
        menu->add_manage_item(new Gtk::CheckMenuItem("Hexadecimal"));

    int base = 10;
    if (numeric_base(symbol.get()) == 16)
    {
        check->set_active();
    }
    else
    {
        base = 16;
    }

    Gtk_CONNECT_1(check, activate, this, &VariablesView::set_numeric_base, base);

    // if live thread (i.e. not loaded from a core file), then
    // add a menu entry for setting watchpoint(s) on this variable
    if (symbol->addr() && symbol->thread() && symbol->thread()->is_live())
    {
        menu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());

        item = menu->add_manage_item(new Gtk::MenuItem("Set Watchpoint..."));
        Gtk_CONNECT(item, activate, Gtk_BIND(set_watchpoint.slot(), symbol));
    }
#ifdef DEBUG
    item = menu->add_manage_item(new Gtk::MenuItem("Type Info"));
    Gtk_CONNECT_1(item, activate, this, &VariablesView::type_info, symbol);
#endif
    menu.release()->popup(event.button, event.time);
}


////////////////////////////////////////////////////////////////
void VariablesView::set_numeric_base(int base)
{
    if (base != base_)
    {
        base_ = base;
        numeric_base_changed();
    }
}


#ifdef DEBUG
////////////////////////////////////////////////////////////////
void VariablesView::type_info(RefPtr<DebugSymbol> symbol)
{
    assert(!symbol.is_null());
    assert(symbol->type());

    Pipe pipe;
    OutputDialog output("Type Info", pipe.output());
    describe_type(symbol, pipe.input());
    assert(get_toplevel());
    Gtk::Window& top = dynamic_cast<Gtk::Window&>(*get_toplevel());
    output.set_transient_for(top);

    output.run(this);
}
#endif // DEBUG


////////////////////////////////////////////////////////////////
bool VariablesView::on_cell_edit(CELL_EDIT_PARAM)
{
    return on_cell_edit_vfunc(path, ncol, old, s);
}


////////////////////////////////////////////////////////////////
bool VariablesView::on_cell_edit_vfunc(CELL_EDIT_PARAM)
{
    Gtk::CTree::Row row = CHKPTR(ctree_)->row(path);

    if (void* data = row.get_data())
    {
        RefPtr<DebugSymbol> sym(reinterpret_cast<DebugSymbol*>(data));

        if (sym->type())
        {
            return edit.emit(sym, s);
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
bool VariablesView::update(RefPtr<Thread>)
{
    return !updatePending_;
}


////////////////////////////////////////////////////////////////
bool VariablesView::get_text_at_pointer(
        Gtk::Widget&    wid,
        double          x,
        double          y,
        Gtk::RowHandle& hrow,
        int&            hcol,
        std::string&    text)
{
    Gtk::CTree& ctree = dynamic_cast<Gtk::CTree&>(wid);

    Gtk::RowHandle nrow; int ncol = 0;
    if (ctree.get_selection_info((int)x, (int)y, &nrow, &ncol))
    {
        if (void* data = ctree.rows()[nrow].get_data())
        {
            if (DebugSymbol* sym = reinterpret_cast<DebugSymbol*>(data))
            {
                if (const char* tip = sym->tooltip())
                {
                    text = tip;
                    return true;
                }
            }
        }
    }
    return ToolTipTraits<Gtk::CTree>::get_text_at_pointer(wid, x, y, hrow, hcol, text);
}


////////////////////////////////////////////////////////////////
void VariablesView::expand(DebugSymbol& sym)
{
    expand_.insert(SymKey(sym));
}


////////////////////////////////////////////////////////////////
string highlight_changes_color()
{
    static string color = env::get_string("ZERO_HIGHLIGHT_CHANGES", "magenta");
    return color;
}


////////////////////////////////////////////////////////////////
void VariablesView::save_config(Properties& prop, const string& prefix)
{
    prop.set_word((prefix + ".0.width").c_str(), ctree_->column(0).get_width());
    prop.set_word((prefix + ".1.width").c_str(), ctree_->column(1).get_width());
}


////////////////////////////////////////////////////////////////
void VariablesView::restore_settings(Properties& prop, const string& prefix)
{
    ctree_->column(0).set_width(prop.get_word((prefix + ".0.width").c_str(), 100));
    ctree_->column(1).set_width(prop.get_word((prefix + ".1.width").c_str(), 150));
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
