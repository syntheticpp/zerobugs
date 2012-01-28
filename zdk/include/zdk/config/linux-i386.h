#ifndef LINUX386_H__DAAA96C8_4EF4_11DA_861C_00C04F09BBCC
#define LINUX386_H__DAAA96C8_4EF4_11DA_861C_00C04F09BBCC
//
// $Id$
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/config/linux-x86.h"

// on newer kernels we might have the tkill system call available
#define HAVE_TKILL_MAYBE            1

#define X86REG(s,r) static_cast<reg_t>(s.r)
#define X86REG_PTR(s,r) (s)->r

#define MAX_FLOAT_BYTES 24 // long double complex


#endif // LINUX386_H__DAAA96C8_4EF4_11DA_861C_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
