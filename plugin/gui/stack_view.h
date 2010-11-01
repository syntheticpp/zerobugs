#ifndef STACK_VIEW_H__2482D190_5622_4853_ABD5_568B99136E9A
#define STACK_VIEW_H__2482D190_5622_4853_ABD5_568B99136E9A
//
// $Id: stack_view.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "gtkmm/clist.h"
#include "gtkmm/list.h"
#include <vector>
#include "custom_tooltip.h"
#include "zdk/export.h"
#include "zdk/platform.h"


struct StackToolTipTraits;

typedef ToolTipped<Gtk::List, StackToolTipTraits> StackViewBase;

/**
 * Displays the current execution stack
 */
class ZDK_LOCAL StackView : public StackViewBase
{
public:
    StackView();

    virtual ~StackView() throw();

    void update(RefPtr<Thread>);

    /**
     * Get the currently selected frame in the view.
     * @note not to be confused with the homonyme method
     * in StackTrace.
     * @todo: can we use StackTrace::selection instead?
     */
    Frame* selected_frame() const;

    /**
     * Gives signal slots a way of distinguishing between
     * user-driven changes and internal, programmatic changes
     * to the list view.
     */
    bool is_change_programmatic() const { return programmatic_; }

    void clear_data() throw();

    Frame* top() const;

    /**
     * Synchronizes the view with the backTrace_; this is
     * needed since adding items to the Gtk::List should
     * happen in the UI thread, and the updating of the
     * backTrace in the main (debugger) thread.
     */
    void display();

    /**
     * select the specified frame, return false if already selected
     */
    bool show_frame(const Frame*);

    bool up_to_date() const { return upToDate_; }

protected:
    void selection_changed_impl();

private:
    RefPtr<StackTrace> trace_;
    bool upToDate_;
    bool programmatic_;
};


inline Frame* StackView::top() const
{
    return (trace_.is_null() || trace_->size() == 0) ? 0 : trace_->frame(0);
}

#endif // STACK_VIEW_H__2482D190_5622_4853_ABD5_568B99136E9A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
