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
// This file contains the implementation of MainWindow methods that
// have to do with watchpoint management.
//
#include "main_window.h"
#include "edit_watchpoint_dlg.h"
#include "set_watchpoint_dlg.h"
#include "slot_macros.h"
#include "zdk/debug_sym.h"
#include "zdk/shared_string_impl.h"
#include "zdk/zobject_impl.h"
#include "gtkmm/connect.h"

namespace
{
    /**
     * Insert watchpoints. Executes on the main debugger thread.
     */
    class SetWatchPoint : public ZObjectImpl<InterThreadCommand>
    {
    public:
        SetWatchPoint
        (
            MainWindow& wnd,
            Debugger& debugger,
            RefPtr<Thread> thread,
            WatchType type,
            addr_t addr,
            RefPtr<DebugSymbol> sym,
            RefPtr<SharedString> value,
            bool global,
            RelType rel = EQ
        )
          : wnd_(wnd)
          , debugger_(debugger)
          , thread_(thread)
          , type_(type)
          , addr_(addr)
          , sym_(sym)
          , value_(value)
          , global_(global)
          , rel_(rel)
        {}

        virtual ~SetWatchPoint() throw() {}

        bool execute()
        {
            bool result = false;

            if (type_ == WATCH_VALUE)
            {
                assert(!sym_.is_null());
                assert(!value_.is_null());

                result = debugger_.break_on_condition(
                    get_runnable(thread_.get()),
                    sym_.get(),
                    rel_,
                    value_.get(),
                    global_);
            }
            else
            {
                assert(addr_);

                result = debugger_.set_watchpoint(
                    get_runnable(thread_.get()),
                    type_,
                    global_,
                    addr_);
            }

            if (!result)
            {
                const std::string message =
                    "All CPU debug registers are in use. "
                    "Could not set memory watchpoint.";

                post_command(
                    &MainWindow::error_message,
                    &wnd_,
                    message);
            }
            return false; /* do not resume execution */
        }

        const char* name() const
        {
            return "set_watchpoint";
        }

    private:
        MainWindow& wnd_;
        Debugger& debugger_;
        RefPtr<Thread> thread_;
        WatchType type_;
        addr_t addr_;
        RefPtr<DebugSymbol> sym_;
        RefPtr<SharedString> value_;
        bool global_;
        RelType rel_;
    };


    /**
     * Helper callback, populates the EditWatchPointDialog.
     */
    class WatchPointEnum
        : public EnumCallback<volatile BreakPoint*>
        , private EnumCallback<BreakPoint::Action*>
    {
    public:
        explicit WatchPointEnum(EditWatchPointDialog& dlg)
            : dlg_(dlg), addr_(0) {}

        /* WatchPoints are implemented as hardware breakpoints */
        void notify(volatile BreakPoint* breakpoint)
        {
            assert(breakpoint);
            addr_ = breakpoint->addr();

            breakpoint->enum_actions(0, this);
        }


        void notify(BreakPoint::Action* action)
        {
            assert(action);
            dlg_.add_action(addr_, action);
        }

    private:
        EditWatchPointDialog& dlg_;
        addr_t addr_; // valid only during enumeration
    };
}


void MainWindow::on_menu_watch()
{
    on_menu_watchpoint(NULL);
}


BEGIN_SLOT(MainWindow::on_menu_watchpoint, (RefPtr<DebugSymbol> dsym))
{
    SetWatchPointDialog dlg(debugger().properties(), dsym);
    dlg.set_memory_watch.connect(Gtk_SLOT(this, &MainWindow::on_mem_watch));

    if (dsym.get())
    {
        dlg.set_value_watch.connect(
            Gtk_BIND(Gtk_SLOT(this, &MainWindow::on_value_watch), dsym));
    }
    dlg.set_transient_for(*this);
    dlg.run();
}
END_SLOT()


BEGIN_SLOT(MainWindow::on_mem_watch, (WatchType type, addr_t addr))
{
    if (addr)
    {
        bool global = debugger().properties()->get_word("watch_all", 1);
        std::clog << __func__ << ": global=" << global << std::endl;

        post_response(
            CommandPtr(new SetWatchPoint(
                *this,
                debugger(),
                current_thread(),
                type,
                addr,
                NULL,
                NULL,
                global)));
    }
}
END_SLOT()


BEGIN_SLOT(MainWindow::on_value_watch,
(
    RelType relop,
    const std::string& value,
    RefPtr<DebugSymbol> dsym
))
{
    RefPtr<SharedString> str = shared_string(value);
    bool global = debugger().properties()->get_word("watch_all", 1);

#ifdef DEBUG
    std::clog << __func__ << ": global=" << global << std::endl;
#endif
    post_response(
        CommandPtr(new SetWatchPoint(
            *this,
            debugger(),
            current_thread(),
            WATCH_VALUE,
            0,
            dsym,
            str,
            global,
            relop)));
}
END_SLOT()


BEGIN_SLOT(MainWindow::on_menu_edit_watchpoints, ())
{
    EditWatchPointDialog dlg(DialogBox::btn_ok_cancel, "Watchpoints");

    /* Populate the dialog's list */
    WatchPointEnum callback(dlg);

    BreakPointManager& mgr =
        interface_cast<BreakPointManager&>(debugger());

    mgr.enum_watchpoints(&callback);

    dlg.set_transient_for(*this);
    std::vector<Gtk::SelectionItem> result = dlg.run(this);

    std::vector<Gtk::SelectionItem>::const_iterator i = result.begin();

    for (; i != result.end(); ++i)
    {
        BreakPoint::Action* action =
            reinterpret_cast<BreakPoint::Action*>(Gtk_USER_DATA(*i));

        /* Currently, the only supported Edit operation is
           deleting the watchpoint action; invoke it on main thread. */
        post_response(command(
            &Debugger::remove_breakpoint_action, &debugger(), action));
    }
}
END_SLOT()
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
