#ifndef ERRORS_H__A3C1DC60_AE00_402D_B0BE_E674A304B535
#define ERRORS_H__A3C1DC60_AE00_402D_B0BE_E674A304B535
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

#include "zdk/export.h"
#include "zdk/stdexcept.h"
#include <string>

struct ZDK_EXPORT ParseError : public std::runtime_error
{
    explicit ParseError(const std::string& m)
        : std::runtime_error(m) { }
};


struct ZDK_EXPORT EvalError : public std::runtime_error
{
    explicit EvalError(const std::string& m)
        : std::runtime_error(m) { }
};


/**
 * Thrown to indicate that the evaluation cannot be
 * completed at this time, because a function in the
 * debugged program was called.
 * The interpreter needs to retry later.
 * @note the implementation arranges for an event
 * (SIGTRAP or SIGSEGV) to occur in the debugee as
 * soon as the call returns.
 */
struct ZDK_EXPORT CallPending : public std::runtime_error
{
     CallPending() : std::runtime_error("call pending") { }
};


#endif // ERRORS_H__A3C1DC60_AE00_402D_B0BE_E674A304B535
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
