#ifndef SYSCALL_PENDING_H__2F29055A_D639_4A70_9A08_32A17A215332
#define SYSCALL_PENDING_H__2F29055A_D639_4A70_9A08_32A17A215332
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: syscall_pending.h 714 2010-10-17 10:03:52Z root $
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
#include "zdk/zerofwd.h"

using Platform::reg_t;

bool thread_syscall_pending(const Thread& thread, reg_t pc);

void thread_cancel_syscall(Thread& thread);

#endif // SYSCALL_PENDING_H__2F29055A_D639_4A70_9A08_32A17A215332
