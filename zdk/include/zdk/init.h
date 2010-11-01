#ifndef INIT_H__8644CC0E_1382_494E_BA67_A0933ABF628D
#define INIT_H__8644CC0E_1382_494E_BA67_A0933ABF628D
//
// $Id: init.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zero.h"

#ifdef __cplusplus
// use C-linkage to avoid C++ mangling issues
extern "C"
{
#endif
/**
 * Parse command line, detect and initialize plug-ins.
 * @return the debugger instance on success, NULL otherwise
 */
ZDK_EXPORT Debugger* debugger_init(int argc, char* argv[]);

/**
 * Begin the execution loop.
 * Calling this function more than once has undefined
 * results. Calling the function without calling debugger_init
 * has undefined results (although the debugger is very likely
 * to run, no plug-in will be loaded). The init and run phases
 * are broken down in separate function calls just to allow
 * the main application to do some additional setup after
 * calling debugger_init.
 * @return true on success, false otherwise
 */
ZDK_EXPORT bool debugger_run(Debugger*);

#ifdef __cplusplus
}
#endif
#endif // INIT_H__8644CC0E_1382_494E_BA67_A0933ABF628D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
