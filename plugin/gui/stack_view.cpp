//
// $Id: stack_view.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <iomanip>
#include <iostream>
#include <sstream>
#include "zdk/thread_util.h"
#include "zdk/utility.h"
#include "zdk/zero.h"
#include "generic/temporary.h"
#include "dharma/symbol_util.h"
#include "gtkmm/flags.h"
#include "gtkmm/list.h"
#include "gui.h"
#include "stack_view.h"


using namespace std;


struct StackToolTipTraits
{
    // hack: memorize the last hovered item, to minimize flicker
    static Gtk::ListItem* item_;

    static bool get_text_at_pointer(
        Gtk::Widget&    widget,
        double          x,
        double          y,
        Gtk::RowHandle& hrow,
        int&            hcol,
        string&         text)
    {
        text.clear();
#ifdef GTKMM_2
        StackView& list = dynamic_cast<StackView&>(widget);
        if (!list.up_to_date())
        {
            return false;
        }
        if (list.items().empty())
        {
            return false;
        }

        Gtk::TreeModel::Path path;
        Gtk::TreeView::Column* col = NULL;
        int cellX = 0, cellY = 0;
        if (list.get_path_at_pos(static_cast<int>(x),
                                 static_cast<int>(y),
                                 path, col, cellX, cellY))
        {
            Gtk::TreeModel::Row row = *list.get_iter(path);
            if (Gtk::ListItem* item = row[list.user_data_col()])
            {
                if (item == item_)
                {
                    return false;
                }
                item_ = item;
                Frame* frame = reinterpret_cast<Frame*>(item->user_data());
                if (frame)
                {
                    ostringstream os;
                    os << "Frame #" << frame->index() << ":";
                    os << " ";
                    os << "PC="<< (void*)frame->program_count();
                    os << " ";
                    os << "FP=" << (void*)frame->frame_pointer();
                    os << " ";
                    os << "SP=" << (void*)frame->stack_pointer();
                    text = os.str();
                }
            }
        }
#endif // GTKMM_2

        return !text.empty();
    }
};


Gtk::ListItem* StackToolTipTraits::item_ = NULL;



StackView::StackView() : upToDate_(false), programmatic_(false)
{
    set_selection_mode(Gtk_FLAG(SELECTION_SINGLE));
}



StackView::~StackView() throw()
{
}



void StackView::selection_changed_impl()
{
    if (!programmatic_ && trace_)
    {
        Gtk::List::SelectionList& sel = Gtk::List::selection();
        if (!sel.empty())
        {
            const int nrow = this->child_position(**sel.begin());
            if (nrow != -1)
            {
                trace_->select_frame(nrow);
            }
        }
    }
    StackViewBase::selection_changed_impl();
}


Frame* StackView::selected_frame() const
{
    return trace_ ? trace_->selection() : NULL;
}



/**
 * @note runs on main thread
 */
void StackView::update(RefPtr<Thread> thread)
{
    RefPtr<StackTrace> trace;

    if (thread && thread_is_attached(*thread))
    {
        // this also has the nice side-effect of priming the regs:
        trace = thread->stack_trace(UINT_MAX);

        assert(trace);
    }

    if (trace != trace_)
    {
        trace_ = trace;
        upToDate_ = false; // trace_ and widget are out of sync now

        if (trace_)
        {
            //
            // prime symbols info on main thread
            //
            for (size_t i = 0; i != trace_->size(); ++i)
            {
                if (Symbol* sym = trace_->frame(i)->function())
                {
                    sym->line();
                }
            }
        }
    }
}



void StackView::clear_data() throw()
{
    upToDate_ = false;
    trace_.reset();
}


///
/// @note runs on UI thread
///
void StackView::display()
{
    if (upToDate_)
    {
        return; // data and view already synchronized -- bail out
    }
    // inhibit on_selection_changed_impl
    Temporary<bool> setValue(programmatic_, true);

    items().clear();
    assert(items().empty());

    size_t n = trace_.get() ? trace_->size() : 0;

    if (n > 1024)
    {
        clog << "Stack depth is too large (" << n;
        clog << " frames), limiting to 1024" << endl;
        n = 1024;
    }
    for (size_t i = 0; i != n; ++i)
    {
        RefPtr<Frame> frame = trace_->frame(i);
        assert(frame);

        const addr_t pc = frame->program_count();
        ostringstream outs;

    #if __PPC__ // until I figure how to get the VDSO symbol
        if (frame->is_signal_handler())
        {
            outs << "0x" << hex << setw(2 * sizeof(addr_t));
            outs << setfill('0') << pc << " (SIGNAL HANDLER)";
        }
        else
    #endif
        {
            print_symbol(outs, pc, frame->function());
        }
        Gtk::ListItem* item = manage( new Gtk::ListItem(outs.str()) );
        item->set_user_data(frame.get());
        add(*item);
    }
    if (trace_)
    {
        if (Frame* frame = trace_->selection())
        {
            assert(frame->index() < items().size());
            select_item(frame->index());
        }
    }
    show_all();
    upToDate_ = true;
}



bool StackView::show_frame(const Frame* frame)
{
    if (!frame || trace_.is_null() || items().empty())
    {
        return false;
    }
    const size_t n = trace_->size();
    //assert(n == items().size());
    const size_t m = min(items().size(), n);

    for (size_t i = 0; i != m; ++i)
    {
        if (trace_->frame(i) == frame)
        {
            Temporary<bool> setFlag(programmatic_, true);
            select_item(i);
            trace_->select_frame(i);

            return true;
        }
    }

    return false;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
