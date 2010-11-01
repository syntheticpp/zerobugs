#ifndef CPROLOGUE_H__A9544D91_034D_46C8_B894_C6654BFB3C83
#define CPROLOGUE_H__A9544D91_034D_46C8_B894_C6654BFB3C83
//
// $Id: cprologue.h 714 2010-10-17 10:03:52Z root $
//
// Prototype of helper functions used by the stack unwinder.
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

struct Frame;
struct Thread;
class FrameImpl;

bool has_c_prologue32(const Thread&, FrameImpl&);

bool has_c_prologue32(const Thread&, const Frame&);
bool has_c_prologue64(const Thread&, const Frame&);

#endif // CPROLOGUE_H__A9544D91_034D_46C8_B894_C6654BFB3C83
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
