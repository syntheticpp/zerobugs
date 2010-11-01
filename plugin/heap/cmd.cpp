// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: cmd.cpp 714 2010-10-17 10:03:52Z root $
//
#include <stdexcept>
#include "cmd.h"
#include "dharma/properties.h"
#include "generic/temporary.h"
#include "target/target.h"
#include "zdk/zero.h"
#include "heap.h"


using namespace std;


HeapMonitorCommand::HeapMonitorCommand
(
    HeapPlugin* plugin,
    const string& name,
    const string& help,
    const char* stockID
)
 : plugin_(plugin)
 , prop_(new PropertiesImpl)
 , name_(name)
 , help_(help)
 , args_(NULL)
{
    prop_->set_string("type", "tool");
    prop_->set_string("tool", "E_xtras");
    prop_->set_string("stock", stockID);
    prop_->set_string("tooltip", help_.c_str());
}



HeapMonitorCommand::~HeapMonitorCommand() throw()
{
}


bool HeapMonitorCommand::execute(Thread* thread,
                          const char* const* args,
                          Unknown2* /* reserved */)
{
    if (thread)
    {
        // set thread and args for the scope of this command
        Temporary<RefPtr<Thread> > setThread(thread_, thread);
        Temporary<const char* const*> setArgs(args_, args);

        if (Target* target = thread->target())
        {
            return target->accept(this);
        }
    }
#ifdef DEBUG
    else
    {
        clog << __func__ << ": no thread" << endl;
    }
#endif
    return false;
}


////////////////////////////////////////////////////////////////
//
//  CONCRETE COMMANDS
//
HeapMonitorOn::HeapMonitorOn(HeapPlugin* plugin)
 : HeapMonitorCommand(plugin,
                "Heap: Off",
                "Turn heap monitoring on",
                "gtk-no")
{
    prop_->set_word("separator", 1);
}


bool HeapMonitorOn::visit(LinuxLiveTarget& target)
{
    plugin_->enable(true);
    return false;
}


HeapMonitorOff::HeapMonitorOff(HeapPlugin* plugin)
 : HeapMonitorCommand(plugin,
                "Heap: On",
                "Turn heap monitoring off",
                "gtk-yes")
{
}


bool HeapMonitorOff::visit(LinuxLiveTarget& target)
{
    plugin_->enable(false);
    return false;
}


HeapMonitorClear::HeapMonitorClear(HeapPlugin* plugin)
 : HeapMonitorCommand(plugin,
                "Heap Clear",
                "Clear heap monitoring internal books",
                "gtk-clear")
{
}


bool HeapMonitorClear::visit(LinuxLiveTarget& target)
{
    throw runtime_error("not implemented");
    return false;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
