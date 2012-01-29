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

#include "zdk/init.h"
#include "zdk/log.h"
#include <iostream>
#include <sstream>
#include "dharma/environ.h"
#include "dharma/object_manager.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"
#include "debugger_shell.h"
#include "target/init.h"
#include <iostream>


using namespace std;

static auto_ptr<DebuggerShell>& debugger()
{
    static auto_ptr<DebuggerShell> dbgPtr(new DebuggerShell);
    return dbgPtr;
}


Debugger* debugger_init(int argc, char* argv[])
{
    uid_t owner = getuid();
    string path = debugger()->get_config_path(&owner);
    path += "log";
    // initalize log file as owner, not as root if running under sudo
    {   sys::ImpersonationScope impersonate(owner);
        Log::init(path.c_str());
    }

#ifdef DEBUG_OBJECT_LEAKS
    ObjectManager::instance();
#endif
    init_core_target();
    init_live_target();

    try
    {
        ExecArg args;

        // read the properties to make the saved history available
        debugger()->properties();

        // Parse command line arguments
        if (!debugger()->initialize(argc, argv, args))
        {
            return 0;
        }
        if ((debugger()->options() & Debugger::OPT_NO_BANNER) == 0)
        {
            debugger()->print_banner();
        }
        cout << "Detecting plugins...\n";
        debugger()->scan_plugins("ZERO_PLUGIN_PATH");

        // save options: so that refreshing the properties
        // does not change values possibly changed by initialize()
        const uint64_t opts = debugger()->options();

        // plugins might have installed streamable objects factories
        debugger()->refresh_properties();
        debugger()->set_options(opts);

        debugger()->set_default_signal_policies();
        debugger()->restore_step_over_properties();

        debugger()->attach_or_exec(args);

        return debugger().get();
    }
    catch (const exception& e)
    {
        cerr << __func__ << ": " << e.what() << endl;
    }
    catch (...)
    {
        cerr << __func__ << ": unknown exception\n";
    }
    return NULL;
}


bool debugger_run(Debugger* debugger)
{
    bool result = false;
    try
    {
        static_cast<DebuggerShell*>(debugger)->run();
        result = true;
    }
    catch (const exception& e)
    {
        cerr << __func__ << ": "  << e.what() << endl;
    }
    catch (...)
    {
        cerr << __func__ << ": unknown exception\n";
    }
    if (!result)
    {
        static_cast<DebuggerShell*>(debugger)->shutdown();
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
