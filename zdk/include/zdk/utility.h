#ifndef UTILITY_H__08841FAF_44F5_4187_8632_9EC2A28E725F
#define UTILITY_H__08841FAF_44F5_4187_8632_9EC2A28E725F
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

#include <iosfwd>

#include "zdk/stack.h"
#include "generic/state_saver.h"


std::ostream& operator<<(std::ostream& os, const Frame& frame);

#endif // UTILITY_H__08841FAF_44F5_4187_8632_9EC2A28E725F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

