// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: attr_def.h 714 2010-10-17 10:03:52Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

DECL_RPC_ATTR(none)

DECL_RPC_ATTR(err_errno)
DECL_RPC_ATTR(err_what)

DECL_RPC_ATTR(exec_cmd)
DECL_RPC_ATTR(exec_env)
DECL_RPC_ATTR(exec_pid)

DECL_RPC_ATTR(kill_pid)
DECL_RPC_ATTR(kill_sig)
DECL_RPC_ATTR(kill_all)

/* ptrace */
DECL_RPC_ATTR(ptrace_req)
DECL_RPC_ATTR(ptrace_pid)
DECL_RPC_ATTR(ptrace_addr)
DECL_RPC_ATTR(ptrace_data)

DECL_RPC_ATTR(ptrace_bits)
DECL_RPC_ATTR(ptrace_ret)

/* initial handshake */
DECL_RPC_ATTR(remote_byte_order)
DECL_RPC_ATTR(remote_system)
DECL_RPC_ATTR(remote_sysver)
DECL_RPC_ATTR(remote_word_size)

/* remote IO */
DECL_RPC_ATTR(rio_file)
DECL_RPC_ATTR(rio_op)
DECL_RPC_ATTR(rio_data)

/* waitpid */
DECL_RPC_ATTR(wait_opt)
DECL_RPC_ATTR(wait_pid)
DECL_RPC_ATTR(wait_stat)

