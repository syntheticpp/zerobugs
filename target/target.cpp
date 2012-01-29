// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/stdexcept.h"
#include "dharma/environ.h"
#include "dharma/process_name.h"
#include "debugger_base.h"  // verbose()
#include "target_factory.h"
#include <fstream>

using namespace std;


TargetPtr Target::new_core_target(debugger_type& dbg, const char* protocol)
{
    TargetPtr target =
        TheTargetFactory::instance().create_target(
            TargetFactory::NativeOS,
            __WORDSIZE,
            false,
            protocol ? protocol : "",
            dbg);
    if (!target)
    {
        throw runtime_error("core target could not be instantiated");
    }
    return target;
}


TargetPtr Target::new_live_target(debugger_type& dbg, const char* targetParam)
{
    string protocol;

    if (targetParam)
    {
        if (const char* p = strstr(targetParam, "://"))
        {
            protocol.assign(targetParam, distance(targetParam, p));
            targetParam = p + 3;
        }
    }

    TargetPtr target =
        TheTargetFactory::instance().create_target(
            TargetFactory::NativeOS,
            __WORDSIZE,
            true,
            protocol.c_str(),
            dbg);
    if (!target)
    {
        throw runtime_error(protocol + " target could not be instantiated");
    }
    target->init(targetParam);
    return target;
}


/**
 * Allow the user to specify where /proc is mounted. This will
 * come in handy when I do more FreeBSD porting: FBSD allows
 * mounting of linprocfs under user-specified mounting point
 */
const string& Target::procfs_root() const
{
    if (procfs_.empty())
    {
        procfs_ = env::get("ZERO_PROCFS", "/proc/");
        if (procfs_.empty())
        {
            procfs_ = "/proc/";
        }
        else if (procfs_[procfs_.size() - 1] != '/')
        {
            procfs_ += '/';
        }
    }
    return procfs_;
}


RefPtr<SharedString> Target::process_name(pid_t pid) const
{
    if (pid)
    {
        return shared_string(realpath_process_name(pid));
    }
    else if (processName_.is_null())
    {
        if (const Process* proc = process())
        {
            if (const char* name = proc->name())
            {
                string path(name);
                debugger().map_path(proc, path);

                processName_ = shared_string(path);
            }
        }
    }
    return CHKPTR(processName_);
}


const char* Target::id() const
{
    return "";
}


string Target::thread_name(pid_t tid) const
{
    return process_name(tid)->c_str();
}


auto_ptr<istream> Target::get_ifstream(const char* filename) const
{
    assert(filename);
    return auto_ptr<istream>(new ifstream(filename));
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
