#ifndef EXEC_H__B8DC3767_419A_461E_9F75_6A1A8D5707EA
#define EXEC_H__B8DC3767_419A_461E_9F75_6A1A8D5707EA
//
// $Id: exec.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sys/types.h>  // pid_t
#include <deque>
#include <string>

/**
 * Forks a child process, then calls execvp in the child
 * passing it the arguments from argv. The executed
 * process' output is optionally redirected to the given
 * file descriptor.
 * Returs the id of the child process.
 * @note it is the caller's responsibility to waitpid()
 */
pid_t exec(
    const std::string& fileName,
    const std::deque<std::string>& argv,
    int redirectOutputFildes = 0);


#endif // EXEC_H__B8DC3767_419A_461E_9F75_6A1A8D5707EA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
