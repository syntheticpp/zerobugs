#ifndef VARIABLES_VIEW_H__1EBC4150_7B6D_445D_86D5_B259C7C18EA7
#define VARIABLES_VIEW_H__1EBC4150_7B6D_445D_86D5_B259C7C18EA7
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

#include <set>
#include <map>
#include "generic/lock_ptr.h"
#include "gtkmm/ctree.h"
#include "gtkmm/events.h"
#include "zdk/debug_sym.h"
#include "zdk/debug_symbol_list.h"
#include "zdk/mutex.h"
#include "custom_tooltip.h"
#include "edit_in_place.h"
#include "var_view_signals.h"
#include "symkey.h"

class Debugger;
class FrameState;
class Thread;


CLASS VariablesView : public VariablesViewSignals
                    , public DebugSymbolEvents
{
    // memorize previous values
    typedef std::map<SymKey, RefPtr<SharedString> > DebugValuesMap;
    typedef std::map<SymKey, RefPtr<DebugSymbol> > DebugSymbolMap;

public:

BEGIN_INTERFACE_MAP(VariablesView)
    INTERFACE_ENTRY(DebugSymbolEvents)
END_INTERFACE_MAP()

    explicit VariablesView(Debugger&);

    virtual ~VariablesView();

    Gtk::CTree::Column column(size_t ncol) const
    {
        return ctree_->column(ncol);
    }

    /**
     * Updates the data (the list of debug symbols), but does not
     * display them yet (because it is called from the main thread).
     * The display() method will take care of sync-ing the visual
     * representation with the internal data.
     *
     * @see LocalsView, ExprEvalView
     */
    virtual bool update(RefPtr<Thread>) = 0;

    virtual bool notify(DebugSymbol*);
    virtual void symbol_change(DebugSymbol* sym, DebugSymbol* old);

    /**
     * Synchronize the view with the internal data. @see update
     */
    virtual void display(bool force = false);

    void reset(bool keepExpand = false);

    /**
     * Get the numeric base to use for displaying
     * integral variables.
     */
    int numeric_base(const DebugSymbol* = 0) const { return base_; }

    /**
     * Set the numeric base for displaying integral variables.
     */
    void set_numeric_base(int base);

    /**
     * DEBUG builds only: popup a window that displays type information
     * for the given symbol (and what classes are used internally to
     * represent type info).
     */
    void type_info(RefPtr<DebugSymbol>);

    DebugSymbolList debug_symbols() const
    {
        Lock<Mutex> lock(mx_);
        return debugSymbols_;
    }

    void restore_state(const Frame*) volatile;

    static bool get_text_at_pointer(
        Gtk::Widget&    wid,
        double          x,
        double          y,
        Gtk::RowHandle& hrow,
        int&            hcol,
        std::string&    text);

    void expand(DebugSymbol&);
    bool is_expanding(DebugSymbol*) const;

    void save_config(Properties&, const std::string&);
    void restore_settings(Properties&, const std::string&);

private:
    /*** internal CTree slots ***/
    void on_expand(Gtk::CTree::Row);

    void on_collapse(Gtk::CTree::Row);

    bool on_cell_edit(CELL_EDIT_PARAM);

    class EnumChildrenHelper;
    friend class EnumChildrenHelper;

    void add_symbol(Gtk::CTree::RowList, RefPtr<DebugSymbol>, bool ref = false);
    void add_symbol(Gtk::CTree::Row, RefPtr<DebugSymbol>, RefPtr<SharedString>);

    static RefPtr<SharedString> validate(const RefPtr<DebugSymbol>&);

protected:
    event_result_t on_button_press_event(GdkEventButton*);

    virtual void clear_data(bool keepExpand = false) throw();

    virtual void popup_menu
      (
        GdkEventButton&     rightClickEvent,
        RefPtr<DebugSymbol> symbolAtCursor
      );

    void clear_symbols()
    {
        debugSymbols_.clear();
        isStaleView_ = true;
    }

    volatile Mutex& mutex() volatile { return mx_; }

    /**
     * Restores the list of expanded variables, the old values
     * and optionally the symbols in view
     */
    void restore_state(const RefPtr<FrameState>&, bool = false) volatile;

    void save_state(FrameState&) volatile;

    void set_is_stale_view(bool f) { isStaleView_ = f; }
    bool is_stale_view() const { return isStaleView_; }

    Gtk::CTree::Row add_row(const char* = NULL,
                            const char* = NULL,
                            const char* = NULL);

    void set_column_editable(int ncol, bool mode)
    {
        ctree_->set_column_editable(ncol, mode);
    }

    virtual bool on_cell_edit_vfunc(CELL_EDIT_PARAM);

    void emit_sym_read(const RefPtr<DebugSymbol>&);
    bool emit_sym_filt(RefPtr<DebugSymbol>&);

private:
    /* the widget that displays the symbols */
    typedef EditableInPlace<ToolTipped<Gtk::CTree, VariablesView> > Tree;

    /* the displayed symbols */
    DebugSymbolList debugSymbols_;

    DebugSymbolList keepAlive_;

    /* previous values, we compare the current ones
       with these and highlight the changes */
    DebugValuesMap values_;

    /* symbols substitued by data filter(s) */
    mutable DebugSymbolMap subst_;

    /* a set of variables that need to be expanded */
    mutable std::set<SymKey> expand_;

    typedef std::map<SymKey, SymKey> KeyMap;
    KeyMap keyMap_;

    int base_;  /* numeric base, for integral values */

    mutable Mutex mx_;

    Tree* ctree_;

    bool isStaleView_;
    bool updatePending_;
};

#endif // VARIABLES_VIEW_H__1EBC4150_7B6D_445D_86D5_B259C7C18EA7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
