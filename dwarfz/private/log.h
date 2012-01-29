#ifndef LOG_H__EC55BE0B_3AB7_4013_A161_0CFD126C20C5
#define LOG_H__EC55BE0B_3AB7_4013_A161_0CFD126C20C5
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

#include "zdk/log.h"
#include <iostream>

#define ALWAYS  -1

#define LOG_WARN        dbgout(ALWAYS) << "Warning: "
#define LOG_ERROR       dbgout(ALWAYS) << "Error: "
#define LOG_DEBUG(n)    dbgout(n)


#endif // LOG_H__EC55BE0B_3AB7_4013_A161_0CFD126C20C5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
