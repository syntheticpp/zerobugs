#ifndef ARCH_H__2A34433B_4B08_4F3D_83AB_AC85BCE3C9AC
#define ARCH_H__2A34433B_4B08_4F3D_83AB_AC85BCE3C9AC
//
// $Id: arch.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#if defined(linux) || defined(__linux__)
 #include "arch-linux.h"
#elif defined(__FreeBSD__)
 #include "arch-fbsd.h"
#endif
#endif // ARCH_H__2A34433B_4B08_4F3D_83AB_AC85BCE3C9AC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
