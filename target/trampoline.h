#ifndef TRAMPOLINE_H__76B14953_7C6D_41B4_B455_9D6FE3554435
#define TRAMPOLINE_H__76B14953_7C6D_41B4_B455_9D6FE3554435
//
// $Id: trampoline.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
class Frame;
class FrameImpl;
class Thread;

/**
 * Check for signal handler frame
 */
bool check_trampoline_frame32(const Thread&, FrameImpl&);
bool check_trampoline_frame64(const Thread&, FrameImpl&);

#endif // TRAMPOLINE_H__76B14953_7C6D_41B4_B455_9D6FE3554435
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
