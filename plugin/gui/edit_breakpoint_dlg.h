#ifndef EDIT_BREAKPOINT_DLG_H__F02B1370_8144_4D1D_B985_48C07566A5F5
#define EDIT_BREAKPOINT_DLG_H__F02B1370_8144_4D1D_B985_48C07566A5F5
//
// $Id: edit_breakpoint_dlg.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/zero.h"
#include "select_dialog.h"

namespace Gtk
{
    class CheckButton;
    class Box;
    class Label;
    class List;
    class SpinButton;
}

class BreakPointInfo; // defined in the .cpp
class CheckListItem;
class TextEntry;

struct BreakPointSite
{
    explicit BreakPointSite(const volatile BreakPoint&);

    addr_t  addr_;  // code address of the breakpoint
    pid_t   tid_;   // thread task ID, or zero if applies to all
};


/**
 * A dialog for editing user breakpoints. Displays breakpoints
 * in a list, they can be removed, enabled or disabled. The OK
 * button commits all changes. Canceling the dialog leaves the
 * breakpoints unchanged.
 * @note: breakpoints set by the debugger for internal purposes
 * are not shown in the list.
 */
class EditBreakPointDialog : public SelectDialog
                           , EnumCallback<volatile BreakPoint*>
                           , EnumCallback<SymbolTable*>
                           , EnumCallback2<BreakPoint*, const SymbolTable*>
{
    typedef EditBreakPointDialog Dialog;
    typedef std::vector<BreakPointSite> BreakPointSiteations;

public:
    EditBreakPointDialog(Debugger&, const Thread*);

    ~EditBreakPointDialog();

    ButtonID run(const Gtk::Widget*);

    /// Breakpoints need to be manipulated from the main debugger
    /// thread -- the one that has called ptrace(PTRACE_ATTACH).
    /// The dialog just emits the signals. They get translated
    /// into debugger calls and marshaled to the main thread by
    /// the main window.
    SigC::Signal1<void, const BreakPointSiteations&> delete_breakpoints;

    SigC::Signal1<void, addr_t> breakpoint_modified;

    /// These signals avoid a compile-time dependency between
    /// the breakpoints dialog and the code view -- the signal
    /// is dispatched to the code-view widget by the main
    /// window, who depends on all other widgets anyway.
    SigC::Signal1<void, RefPtr<BreakPoint> > show_code;

private:
    int on_click(GdkEventButton*, CheckListItem*);

    int on_key(GdkEventButton*, CheckListItem*);

    void on_toggled(CheckListItem*);

    void on_delete();

    void on_enable(bool);

    void on_selection_changed_impl();

    void on_reset_count();

    void popup_menu(const GdkEventButton&, CheckListItem&);

    void update_current_hits(unsigned long);

    // helper called from ctor
    void populate_breakpoint_list(Debugger&);

    void commit_breakpoint_changes();

    void delete_deferred_breakpoints();

    // --- EnumCallback interfaces implementation --------------
    void notify(volatile BreakPoint*);

    void notify(BreakPoint*, const SymbolTable*);

    void notify(SymbolTable*);

private:
    typedef std::vector<RefPtr<BreakPointInfo> > BreakPointInfoList;

    BreakPointInfoList infoList_;

    RefPtr<BreakPointInfo> current_;

    BreakPointSiteations deletionList_;

    std::vector<RefPtr<Symbol> > deferredDeletionList_;

    Gtk::Box* conditionsBox_;

    Gtk::Label* hits_;

    Gtk::SpinButton* activation_;

    Gtk::CheckButton* autoReset_;

    TextEntry* condition_;

    pid_t pid_;
};
#endif // EDIT_BREAKPOINT_DLG_H__F02B1370_8144_4D1D_B985_48C07566A5F5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
