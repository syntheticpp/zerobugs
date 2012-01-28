#ifndef UNWIND_STACK_FRAME_H__CEE4AF81_91D5_4046_9D7D_A06C86A64ED7
#define UNWIND_STACK_FRAME_H__CEE4AF81_91D5_4046_9D7D_A06C86A64ED7
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

#include "zdk/ref_ptr.h"

class Frame;
class Thread;

RefPtr<Frame> unwind_stack_frame(Thread&, Frame*);


#endif // UNWIND_STACK_FRAME_H__CEE4AF81_91D5_4046_9D7D_A06C86A64ED7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
