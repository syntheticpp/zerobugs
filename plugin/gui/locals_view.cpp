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
#include "generic/lock_ptr.h"
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "frame_state.h"
#include "locals_view.h"

using namespace std;


LocalsView::LocalsView(Debugger& debugger) : VariablesView(debugger)
{
    // force the main window to refresh this view
    numeric_base_changed.connect(bind(symbol_expand.slot(), (DebugSymbol*)0));
}


static bool is_same_scope(RefPtr<Symbol> f1, RefPtr<Symbol> f2)
{
    bool result = false;

    if (f1.get() && f2.get())
    {
        result = (f1->value() == f2->value());
    }

    return result;
}


bool LocalsView::update(RefPtr<Thread> thread)
{
    Lock<Mutex> lock(this->mutex());
    if (!VariablesView::update(thread))
    {
        return false;
    }
    clear_symbols();
    if (!thread || thread_finished(*thread))
    {
        fun_.reset();
    }
    else if (Debugger* debugger = thread->debugger())
    {
        RefPtr<Frame> frame = thread_current_frame(thread.get());

        if (frame.is_null())
        {
            return false;
        }
        RefPtr<Symbol> fun = frame->function();
        RefPtr<FrameState> state =
            interface_cast<FrameState*>(
                frame->get_user_object(".state"));

        if (!is_same_scope(fun_, fun))
        {
            restore_state(state);
        }
        fun_ = fun;

        debugger->enum_variables(
            thread.get(),
            0,     // name
            0,     //fun.get(),
            this,
            LOOKUP_LOCAL); // do not include globals

        // save the current state
        if (state.is_null())
        {
            state = new FrameState;
            frame->set_user_object(".state", state.get());
        }
        save_state(*state);
    }
    return true;
}


void LocalsView::clear_data(bool keepExpand) throw()
{
    VariablesView::clear_data(keepExpand);
    fun_.reset();
}



// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
