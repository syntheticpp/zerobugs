//
// $Id: breakpoint.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <boost/bind.hpp>
#include <boost/python.hpp>
#include "zdk/get_pointer.h"
#include "zdk/switchable.h"
#include "zdk/zero.h"
#include "breakpoint.h"
#include "locked.h"
#include "marshaller.h"

using namespace std;
using namespace boost;
using namespace boost::python;



typedef vector<RefPtr<BreakPointAction> > ActionList;

namespace
{
    /**
     * Helper class for enumerating breakpoint actions
     */
    class ZDK_LOCAL ActionEnumerator : public EnumCallback<BreakPointAction*>
    {
        ActionList actions_;

        void notify(BreakPointAction* action)
        {
            if (action)
            {
                actions_.push_back(action);
            }
        }

    public:
        typedef ActionList::const_iterator const_iterator;

        const_iterator begin() const { return actions_.begin(); }
        const_iterator end() const { return actions_.end(); }
      /*
        const ActionList& actions() const
        {
            return actions_;
        }
       */
    };
}



static void remove_(RefPtr<BreakPoint> bpnt, const char* actName)
{
    if (Thread* ownerThread = bpnt->thread())
    {
        pid_t pid = 0;
        if (RefPtr<Process> proc = ownerThread->process())
        {
            pid = proc->pid();
        }
        BreakPointManager* bpntMgr =
            interface_cast<BreakPointManager*>(ownerThread->debugger());

        if (bpntMgr)
        {
            const addr_t addr = bpnt->addr();

            pid_t lwpid = ownerThread->lwpid();
            if (bpnt->type() == BreakPoint::GLOBAL)
            {
                lwpid = 0;
            }
            bpntMgr->remove_breakpoint_actions(pid, lwpid, addr, actName);
        }
    }
}


static void breakpoint_remove(BreakPoint* bpnt)
{
    ThreadMarshaller::instance().send_command
      (
        bind(remove_, bpnt, "USER"), "BreakPoint.remove"
      );
}


static void
enum_actions_(RefPtr<BreakPoint> bpnt,
              string name,
              ActionEnumerator* actions)
{
    bpnt->enum_actions(name.empty() ? NULL : name.c_str(), actions);
}


static boost::python::list
enum_actions(BreakPoint* bpnt, const char* name = "")
{
    boost::python::list acts;

    ActionEnumerator actions;
    ThreadMarshaller::instance().send_command
      (
        bind(enum_actions_, bpnt, name, &actions), "BreakPoint.actions"
      );
    ActionList::const_iterator i = actions.begin();
    for (; i != actions.end(); ++i)
    {
        acts.append(*i);
    }
    return acts;
}


BOOST_PYTHON_FUNCTION_OVERLOADS(enum_actions_overload, enum_actions, 1, 2)

////////////////////////////////////////////////////////////////
// BreakPointAction
//
void set_condition(BreakPointAction* act, const char* cond)
{
    if (Switchable* switchable = interface_cast<Switchable*>(act))
    {
        switchable->set_activation_expr(cond);
    }
}

static string get_condition(BreakPointAction* act)
{
    string cond;

    if (Switchable* switchable = interface_cast<Switchable*>(act))
    {
        cond = switchable->activation_expr();
    }
    return cond;
}

static void set_counter(BreakPointAction* act, unsigned long count)
{
    if (Switchable* switchable = interface_cast<Switchable*>(act))
    {
        switchable->set_counter(count);
    }
}

static unsigned long
get_counter(BreakPointAction* act)
{
    unsigned long count = 0;

    if (Switchable* switchable = interface_cast<Switchable*>(act))
    {
        count = switchable->counter();
    }
    return count;
}


/**
 * Set after how many hits to activate the breakpoint
 */
static void
set_activation_counter(BreakPointAction* act,
                       unsigned long count,
                       bool autoReset = true)
{
    Lock<Mutex> lock(python_mutex());

    if (Switchable* switchable = interface_cast<Switchable*>(act))
    {
        switchable->set_activation_counter(count, autoReset);
    }
}

static unsigned long get_activation_counter(BreakPointAction* act)
{
    unsigned long count = 0;

    Lock<Mutex> lock(python_mutex());

    if (Switchable* switchable = interface_cast<Switchable*>(act))
    {
        count = switchable->activation_counter();
    }
    return count;
}


BOOST_PYTHON_FUNCTION_OVERLOADS(set_activation_counter_overload,
                                set_activation_counter, 2, 3)


static void enable(BreakPointAction* act)
{
    Lock<Mutex> lock(python_mutex());

    if (Switchable* switchable = interface_cast<Switchable*>(act))
    {
        switchable->enable();
    }
}


static void disable(BreakPointAction* act)
{
    Lock<Mutex> lock(python_mutex());

    if (Switchable* switchable = interface_cast<Switchable*>(act))
    {
        switchable->disable();
    }
}


////////////////////////////////////////////////////////////////
void export_breakpoint()
{
    class_<BreakPointAction, bases<>, boost::noncopyable>("BreakPointAction",
        "describes an action associated with a breakpoint",
        no_init)

        .def("name", &BreakPointAction::name, "", locked<>())
        .def("set_activation_counter", set_activation_counter,
             set_activation_counter_overload(args("count", "autoReset"),
             "set an activation counter for a breakpoint action;\n"
             "the breakpoint must be hit the specified number of times\n"
             "before the associated action is executed\n"
             "NOTE that if a conditional expression is also in effect,\n"
             "it takes precedence, and the breakpoint is considered \"hit\"\n"
             "only when the condition evaluates to True")
            )
        .def("activation_counter", get_activation_counter, "")
        .def("set_condition", set_condition,
             "set the conditional expression for activating this action",
             locked<>()
            )
        .def("condition", get_condition,
             "return the conditional expression for activating this action, if any",
             locked<>()
            )
        .def("set_counter", set_counter, "force counter to given value",locked<>())
        .def("counter", get_counter,
             "return the number of times that the breakpoint has\n"
             "been hit to date; NOTE: if a conditional expression\n"
             "has been specified, the breakpoint is considered \"hit\" only\n"
             "when the condition evaluates to True (non-zero)",
             locked<>()
            )
        .def("disable", disable)
        .def("enable", enable)
        ;

scope in_BreakPoint =
    class_<BreakPoint, bases<>, boost::noncopyable>("BreakPoint", no_init)
        .def("addr", &BreakPoint::addr,
            "return address of this breakpoint"
            )
        .def("actions", enum_actions, enum_actions_overload(
             args("name"),
             "Return a list of actions associated with this breakpoint")
            )
        .def("is_enabled", &BreakPoint::is_enabled, "", locked<>())
        .def("owner", &BreakPoint::thread,
            "return the thread that owns this breakpoint",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("remove", breakpoint_remove,
            "Remove the breakpoint if it was set interactively by the user;\n"
            "this method has no effect on breakpoints set internally by the\n"
            "debugger engine."
            )
        .def("symbol", &BreakPoint::symbol,
            "return the ELF symbol corresponding to this breakpoint",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("type", &BreakPoint::type, "return the zero.BreakPoint.Type")
        ;

    enum_<BreakPoint::Type>("Type")
        .value("Software", BreakPoint::SOFTWARE)
        .value("Hardware", BreakPoint::HARDWARE)
        .value("Global", BreakPoint::GLOBAL)
        .value("PerThread", BreakPoint::PER_THREAD)
        .value("Any", BreakPoint::ANY)
        ;


    register_ptr_to_python<WeakPtr<BreakPoint> >();
    register_ptr_to_python<RefPtr<BreakPoint> >();
    register_ptr_to_python<RefPtr<BreakPointAction> >();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
