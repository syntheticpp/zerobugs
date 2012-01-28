// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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

#include "zdk/config.h"
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <sys/utsname.h>
#include "zdk/arch.h"
#include "zdk/zero.h"
#include "dharma/syscall_wrap.h"
#include "engine/ptrace.h"
#include "target/target.h"
#include "msg.h"


using namespace std;


bool RPC::Exec::dispatch(InputStream&, OutputStream& output)
{
    const string& cmd = value<exec_cmd>(*this);

    ExecArg args(cmd);
#if DEBUG
    clog << __func__ << " exec: " << cmd << endl;
#endif
    if (pid_t pid = sys::fork())
    {
        value<exec_pid>(*this) = pid;
        output.write_object(name(), this);
#if DEBUG
        clog << __func__ << " exec=" << pid << endl;
#endif
    }
    else
    {
        sys::unmask_all_signals();
        //
        // Note: XTrace::exec() calls PTRACE_TRACEME
        //
        XTrace::exec(args.cstrings(), environ);
    }
    return true;
}



bool RPC::Waitpid::dispatch(InputStream&, OutputStream& output)
{
    int status = 0;
    pid_t pid = value<wait_pid>(*this);
    const int opt = value<wait_opt>(*this);

    pid = sys::waitpid(pid, &status, opt);

    value<wait_pid>(*this) = pid;
    value<wait_stat>(*this) = status;

    output.write_object(name(), this);
    return true;
}



bool RPC::Ptrace::dispatch(InputStream&, OutputStream& out)
{
    const word_t req = value<ptrace_req>(*this);
#if 0 //DEBUG
    clog << __func__ << " ptrace_req=" << (void*)req << endl;
#endif
    XTrace::Arg addr = value<ptrace_addr>(*this);
    XTrace::Arg data = value<ptrace_data>(*this);
    const pid_t pid = value<ptrace_pid>(*this);

    switch (req)
    {
    case PTRACE_TRACEME:
        throw SystemError("ptrace", EIO);
        break;

    case PTRACE_GETREGS:
        bits().resize(sizeof (user_regs_struct));
        data.p_ = &bits()[0];
        break;
    case PTRACE_GETFPREGS:
        bits().resize(sizeof (user_fpregs_struct));
        data.p_ = &bits()[0];
        break;
#if HAVE_STRUCT_USER_FPXREGS_STRUCT
    case PTRACE_GETFPXREGS:
        bits().resize(sizeof (user_fpxregs_struct));
        data.p_ = &bits()[0];
        break;

    case PTRACE_SETFPXREGS:
#endif
    case PTRACE_SETREGS:
    case PTRACE_SETFPREGS:
        data.p_ = &bits()[0];
        break;

    case PTRACE_GETSIGINFO:
        bits().resize(sizeof (siginfo_t));
        data.p_ = &bits()[0];
        break;

    case PTRACE_GETEVENTMSG:
        bits().resize(sizeof (unsigned long));
        data.p_ = &bits()[0];
        break;
    }
    long result = sys::ptrace(__ptrace_request(req), pid, addr.i_, data.i_);
    value<ptrace_ret>(*this) = result;
    out.write_object(name(), this);
    return true;
}



bool RPC::SysInfo::dispatch(InputStream&, OutputStream& out)
{
    value<remote_word_size>(*this) = __WORDSIZE;
    value<remote_byte_order>(*this) = __BYTE_ORDER;

    utsname u;
    if (uname(&u) < 0)
    {
        throw SystemError("uname");
    }
    value<remote_system>(*this) = u.sysname;
    value<remote_sysver>(*this) = u.release;
    out.write_object(name(), this); // write back response
    return true;
}



bool RPC::Kill::dispatch(InputStream&, OutputStream& out)
{
    const pid_t pid = value<kill_pid>(*this);
    const int sig = value<kill_sig>(*this);
    const bool all = value<kill_all>(*this);

    if (all)
    {
        if (::kill(pid, sig) < 0)
        {
            ostringstream err;
            err << "kill";
            if (all) { err << "_all"; }
            err << "(" << pid << ")";
            throw SystemError(err.str());
        }
    }
    else
    {
        if (::kill_thread(pid, sig) < 0)
        {
            throw SystemError("kill_thread");
        }
    }
#if 0 //DEBUG
    clog << "kill: " << pid << ", " << sig << ", " << all << endl;
#endif
    out.write_object(name(), this);
    return true;
}

