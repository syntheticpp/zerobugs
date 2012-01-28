#ifndef ALIGN_H__DB32BEB6_62AC_4D73_B86A_2E071C43239A
#define ALIGN_H__DB32BEB6_62AC_4D73_B86A_2E071C43239A
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

#define align(addr, pow) \
    (((addr) + ((Elf64_Addr) 1 << (pow)) - 1) & ((Elf64_Addr) -1 << (pow)))

#endif // ALIGN_H__DB32BEB6_62AC_4D73_B86A_2E071C43239A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
