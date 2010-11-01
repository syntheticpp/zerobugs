// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: msgcli.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include <stdexcept>
#include "zdk/zero.h"
#include "dharma/syscall_wrap.h"
#include "msg.h"

using namespace std;


RPC::Error::Error(const exception& e)
{
    value<RPC::err_what>(*this) = e.what();
}


RPC::Error::Error(const SystemError& e)
{
    value<RPC::err_what>(*this) = e.what();
    value<RPC::err_errno>(*this) = e.error();
}


bool RPC::Error::dispatch(InputStream&, OutputStream&)
{
    if (word_t e = value<RPC::err_errno>(*this))
    {
        throw SystemError(e, value<RPC::err_what>(*this), false);
    }
    else
    {
        throw runtime_error(value<RPC::err_what>(*this));
    }
    return false;
}



RPC::Exec::Exec(const ExecArg& args, bool, const char* const* /* env */)
{
    value<exec_cmd>(*this) = args.command_line();
}


RPC::Ptrace::Ptrace(__ptrace_request req, word_t pid, word_t addr, word_t data)
{
    value<RPC::ptrace_req>(*this) = req;
    value<RPC::ptrace_pid>(*this) = pid;
    value<RPC::ptrace_addr>(*this) = addr;
    value<RPC::ptrace_data>(*this) = data;
}



RPC::Waitpid::Waitpid(pid_t pid, int opt)
{
    value<wait_pid>(*this) = pid;
    value<wait_opt>(*this) = opt;
}


RPC::RemoteIO::RemoteIO(IOCommand op, const uint8_t* data, size_t size)
{
    value<rio_op>(*this) = op;
    value<rio_data>(*this).assign(data, data + size);
}
