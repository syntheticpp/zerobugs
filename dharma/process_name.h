#ifndef PROCESS_NAME_H__1056609065
#define PROCESS_NAME_H__1056609065
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
#include <new>         // ::std::nothrow_t
#include <sys/types.h> // pid_t
#include <string>

std::string get_process_name(pid_t = 0);
std::string get_process_name(std::nothrow_t, pid_t);

/**
* Returns the realpath to the process image file
*/
std::string realpath_process_name(pid_t = 0);


#endif // PROCESS_NAME_H__1056609065
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
