#ifndef ADJUST_BASE_H__95684845_D150_4358_8260_996CB71D29C9
#define ADJUST_BASE_H__95684845_D150_4358_8260_996CB71D29C9
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

#include "zdk/types.h"
#include "zdk/ref_ptr.h"

/// read an offset (vbase offset or offset-to-top) from
/// a vtable that starts at ADDR
off_t get_vtable_adjustment(Thread&, addr_t addr, off_t offs);

addr_t ZDK_LOCAL adjust_base_to_derived(
    Thread&             thread,
    addr_t              addr,
    const char*         debugFormat,// stabs or dwarf
    const ClassType&    klass,      // potential base class
    const ClassType&    derived);

addr_t ZDK_LOCAL adjust_base_to_derived(
    const DebugSymbol&  dsym,
    const ClassType&    klass,      // potential base class
    const ClassType&    derived);

#endif // ADJUST_BASE_H__95684845_D150_4358_8260_996CB71D29C9
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
