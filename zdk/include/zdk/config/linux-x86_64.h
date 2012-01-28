#ifndef LINUX86_64_H__CE5900A5_B3AC_4971_8FBE_10EB400FDA54
#define LINUX86_64_H__CE5900A5_B3AC_4971_8FBE_10EB400FDA54
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

#include "zdk/config/linux-x86.h"

#define __FRAME_OFFSETS  // as nothing

#define HAVE_TKILL_MAYBE 1

#define X86REG(s,r) static_cast<reg_t>(s.r)
#define X86REG_PTR(s,r) ((s)->r)

#define MAX_FLOAT_BYTES  32

#endif // LINUX86_64_H__CE5900A5_B3AC_4971_8FBE_10EB400FDA54
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
