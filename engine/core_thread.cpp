#include "zdk/zero.h"
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
#include <stdio.h>      // snprintf
#ifdef HAVE_SYS_PROCFS_H
 #include <sys/procfs.h>
#endif
#ifdef HAVE_SYS_WAIT_H
 #include <sys/wait.h>  // W_STOPCODE
#endif
#include <iostream>
#include <sstream>
#include "elfz/public/core_file.h"
#include "zdk/zero.h"
#include "zdk/32_on_64.h"
#include "core_thread.h"
#include "debugger_base.h"
#include "stack_trace.h"
#include "target/target.h"
#include "thread_base.h"


using namespace std;

static void unsupported(const string& func)
{
    static const char msg[] =
        ": operation not available for core files";
    throw logic_error ((func + msg).c_str());
}

#define NOT_SUPPORTED unsupported(__func__)


////////////////////////////////////////////////////////////////
CoreThreadImpl::CoreThreadImpl
(
    Target&             target,
    CorePtr             core,
    size_t              index,
    const prstatus_t&   prstatus,
    SymbolMap*          symbols
)
  : core_(core)
  , target_(&target)
  , index_(index)
  , lwpid_(prstatus.pr_pid)
  , ppid_(0)
  , gid_(0)
  , cursig_(prstatus.pr_cursig)
{
#ifdef HAVE_PRSTATUS_PPID
    ppid_ = prstatus.pr_ppid;
#endif

#ifdef HAVE_PRSTATUS_PGRP
    gid_ = prstatus.pr_pgrp;
#endif
}


////////////////////////////////////////////////////////////////
CoreThreadImpl::~CoreThreadImpl() throw()
{
}


////////////////////////////////////////////////////////////////
pid_t CoreThreadImpl::lwpid() const
{
    return lwpid_;
}


////////////////////////////////////////////////////////////////
pid_t CoreThreadImpl::ppid() const
{
    return ppid_;
}


////////////////////////////////////////////////////////////////
pid_t CoreThreadImpl::gid() const
{
    return gid_;
}


////////////////////////////////////////////////////////////////
unsigned long CoreThreadImpl::thread_id() const
{
    // Q: is there a way to find the thread id in the corefile?
    return 0;
}



////////////////////////////////////////////////////////////////
Runnable::State CoreThreadImpl::runstate() const
{
    return Runnable::ZOMBIE; // any better ideas?
}


////////////////////////////////////////////////////////////////
const char* CoreThreadImpl::filename() const
{
    return core_->process_name().c_str();
}


////////////////////////////////////////////////////////////////
addr_t CoreThreadImpl::ret_addr()
{
    addr_t addr = 0;
    core_->readval(stack_pointer(), addr);

    Platform::after_read(*this, addr);
    return addr;
}


////////////////////////////////////////////////////////////////
addr_t CoreThreadImpl::stack_start() const
{
    NOT_SUPPORTED;
    return 0;
}



////////////////////////////////////////////////////////////////
//
// @return the value stored in the N-th general purpose register
// @todo unit-test
//
reg_t CoreThreadImpl::read_register(int regIndex, bool readFrame) const
{
    if (RefPtr<Target> t = target())
    {
        reg_t r = 0;

        if (t->read_register(*this, regIndex, readFrame, r))
        {
            return r;
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////
int32_t CoreThreadImpl::status() const
{
    return W_STOPCODE(cursig_);
}


////////////////////////////////////////////////////////////////
bool CoreThreadImpl::exited(int* pStatus) const
{
    const int32_t stat = status();
    if (pStatus)
    {
        *pStatus = stat;
    }
    return WIFEXITED(stat);
}


////////////////////////////////////////////////////////////////
int32_t CoreThreadImpl::signal() const
{
    return cursig_;
}

////////////////////////////////////////////////////////////////
void CoreThreadImpl::set_signal(int32_t)
{
    NOT_SUPPORTED;
}


////////////////////////////////////////////////////////////////
bool CoreThreadImpl::single_step_mode() const
{
    return false;
}


////////////////////////////////////////////////////////////////
bool CoreThreadImpl::is_event_pending() const
{
    return false;
}


////////////////////////////////////////////////////////////////
StackTrace* CoreThreadImpl::stack_trace(size_t depth) const
{
    if (!trace_ || (trace_->size() < depth && !trace_->is_complete()))
    {
        trace_ = new StackTraceImpl(*this);
        trace_->unwind(*this, depth);
    }
    return trace_.get();
}


////////////////////////////////////////////////////////////////
size_t CoreThreadImpl::stack_trace_depth() const
{
    size_t depth = 0;

    if (trace_)
    {
        depth = trace_->size();
    }
    return depth;
}


////////////////////////////////////////////////////////////////
DebugSymbol* CoreThreadImpl::func_return_value()
{
    return NULL;
}


////////////////////////////////////////////////////////////////
void CoreThreadImpl::set_user_data(
    const char* key,
    word_t      val,
    bool        replace)
{
    assert(key);

    ThreadBase::set_user_data(key, val, replace);
}


////////////////////////////////////////////////////////////////
bool CoreThreadImpl::get_user_data(const char* key, word_t* val) const
{
    assert(key);

    return ThreadBase::get_user_data(key, val);
}

////////////////////////////////////////////////////////////////
void CoreThreadImpl::set_user_object(
    const char* key,
    ZObject*    object,
    bool        replace)
{
    assert(key);
    ThreadBase::set_user_object(key, object, replace);
}

////////////////////////////////////////////////////////////////
ZObject* CoreThreadImpl::get_user_object(const char* key) const
{
    assert(key);
    return ThreadBase::get_user_object(key);
}


////////////////////////////////////////////////////////////////
// Enumerate general purpose AND FPU, XMM, etc. registers
size_t CoreThreadImpl::enum_cpu_regs(EnumCallback<Register*>* callback)
{
    // general purpose registers
    size_t count = enum_user_regs(callback);

    // FPU and extended regs
    count += enum_fpxregs(callback);

    return count;
}


////////////////////////////////////////////////////////////////
void CoreThreadImpl::detach()
{
    NOT_SUPPORTED;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
