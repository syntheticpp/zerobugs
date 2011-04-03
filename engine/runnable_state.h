#ifndef RUNNABLE_STATE_H__4D3E5DDD_0EE1_4BE3_B8F1_7B905E40B65A
#define RUNNABLE_STATE_H__4D3E5DDD_0EE1_4BE3_B8F1_7B905E40B65A
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: runnable_state.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/platform.h"
#include "zdk/runnable.h"

using Platform::addr_t;
using Platform::reg_t;

BOOST_STATIC_ASSERT(sizeof(pid_t) == sizeof(int32_t));

struct RunnableState
{
    char        state_;
    int32_t     lwpid_;
    int32_t     ppid_;      // parent pid
    int32_t     gid_;       // group id
    int32_t     session_;
    int32_t     tty_;
    int32_t     tpgid_;
    int64_t     flags_;
    uint64_t    usrTicks_;  // jiffies in user mode
    uint64_t    sysTicks_;  // jiffies in system moe

    uint64_t    vmemSize_;  // virtual mem size in bytes
    uint64_t    stackStart_;

    int32_t     euid_;
    int32_t     ruid_;
    int32_t     suid_;

    explicit RunnableState(pid_t lwpid)
        : state_(Runnable::RUNNING)
        , lwpid_(lwpid)
        , ppid_(0)
        , gid_(0)
        , session_(-1)
        , tty_(-1)
        , tpgid_(0)
        , flags_(0)
        , usrTicks_(0)
        , sysTicks_(0)
        , vmemSize_(0)
        , stackStart_(0)
        , euid_(-1)
        , ruid_(-1)
        , suid_(-1)
    { }
};


#endif // RUNNABLE_STATE_H__4D3E5DDD_0EE1_4BE3_B8F1_7B905E40B65A
