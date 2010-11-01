//
// $Id: watch_view.cpp 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/thread_util.h"
#include "zdk/variant.h"
#include "typez/public/debug_symbol.h"
#include "gui.h"
#include "utils.h"
#include "watch_view.h"

using namespace std;


WatchView::WatchView(Debugger& debugger)
    : Base(debugger)
    , pending_(false)
{
    column(0).set_width(200);
    column(1).set_width(350);
    set_column_editable(0, true);

    column(0).set_passive();
    column(1).set_passive();
    column(2).set_passive();

    numeric_base_changed.connect(bind(symbol_expand.slot(), (DebugSymbol*)0));
}


/**
 * @note runs on the UI thread
 */
void WatchView::display(bool force)
{
    Lock<Mutex> lock(this->mutex());
    assert_ui_thread();
    /* clog << __func__ << ": " << debug_symbols().size()
                     << " is_visible=" << is_visible()
                     << " is_stale_view=" << is_stale_view()
                     << " force=" << force << endl; */
    bool stale = is_stale_view();
    VariablesView::display(force);

    if (is_visible() && stale)
    {
        for (ErrMap::const_iterator i = errMap_.begin();
             i != errMap_.end(); ++i)
        {
            add_row(i->first.c_str(), i->second.c_str()).set_foreground(Gdk_Color("red"));
        }
        add_row(); // add an empty, editable row
    }
}


/**
 * @note runs on the main debugger thread
 */
bool WatchView::update(RefPtr<Thread> thread)
{
    Lock<Mutex> lock(this->mutex());
    assert_main_thread();

    if (!VariablesView::update(thread))
    {
        return true;
    }
    if (!thread || thread_finished(*thread))
    {
        workSet_.clear();
        return true;
    }
    else
    {
        if (pending_)
        {
            pending_ = false;
            set_is_stale_view(true);
        }
        else if (workSet_.empty())
        {
            clear_symbols();
            errMap_.clear();
            workSet_ = watchSet_;
        }
        else
        {
            set_is_stale_view(true);
        }

        for (WatchSet::const_iterator i = workSet_.begin(); i != workSet_.end();)
        {
            events_ = EvalEvents::create(thread.get());
            connect_signals();

            current_ = *i;
            workSet_.erase(i++);

            if (!evaluate(current_.c_str(), 0, CHKPTR(events()), numeric_base()))
            {
                pending_ = true;
                return false;
            }
        }
        events_.reset();
    }
    return true;
}


/**
 * @param path
 * @param ncol
 * @param old
 * @param s
 */
bool WatchView::on_cell_edit_vfunc(CELL_EDIT_PARAM)
{
    Lock<Mutex> lock(this->mutex());
    if (ncol > 0)
    {
        return VariablesView::on_cell_edit_vfunc(path, ncol, old, s);
    }
    if (!old.empty())
    {
        watchSet_.erase(old);
    }
    if (!s.empty())
    {
        watchSet_.insert(s);
    }
    symbol_expand.emit(NULL); // force update
    return true;
}


/**
 * @note runs on main thread
 */
void WatchView::on_done(const Variant& var)
{
    Lock<Mutex> lock(this->mutex());
    assert_main_thread();

    if (DebugSymbol* sym = var.debug_symbol())
    {
        RefPtr<SharedString> name = shared_string(current_);
        interface_cast<DebugSymbolImpl&>(*sym).set_name(name.get());

        notify(sym);
    }
}


/**
 * @note runs on main thread
 */
bool WatchView::on_error(string msg)
{
    Lock<Mutex> lock(this->mutex());
    assert_main_thread();

    errMap_[current_] = msg;
    return false;
}


void WatchView::save_config()
{
    if (RefPtr<WatchList> watchList = watchList_.ref_ptr())
    {
        watchList->clear();
        for (WatchSet::const_iterator i = watchSet_.begin(); i != watchSet_.end(); ++i)
        {
            watchList->add(i->c_str());
        }
    #if DEBUG
        clog << watchSet_.size() << " watch(es) saved.\n";
    #endif
    }
}


void WatchView::restore(Thread& thread)
{
    class Restore : public EnumCallback<const char*>
    {
        WatchSet& watches_;

    public:
        explicit Restore(WatchSet& watchSet) : watches_(watchSet)
        {
        }

        void notify(const char* watch)
        {
            watches_.insert(watch);
        }
    };


    if (RefPtr<Properties> props = get_history_properties(thread))
    {
        if (RefPtr<Process> process = thread.process())
        {
            if (process->enum_threads() > 1)
            {
                return; // do it only for the first thread
            }
            watchList_ = interface_cast<WatchList>(process);
            if (RefPtr<WatchList> watchList = watchList_.ref_ptr())
            {
                watchSet_.clear();
                Restore restore(watchSet_);

                watchList->foreach(&restore);
            }
        }
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
