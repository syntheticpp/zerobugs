#ifndef FRAME_HANDLER_H__2DAA3AA7_BB39_4F5B_91D7_2E6947372B0D
#define FRAME_HANDLER_H__2DAA3AA7_BB39_4F5B_91D7_2E6947372B0D
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

#include "zdk/enum.h"
#include "zdk/platform.h"
#include "zdk/unknown2.h"

struct Frame;
struct Thread;

/**
 * The purpose of frame handlers is to support stack unwinding
 * when the frame pointer register is being used as a general
 * purpose register. This is the case of executables compiled
 * with GCC using the -fomit-frame-pointer option, for example.
 * The stack unwinding logic looks for any registered frame
 * handlers first, then it defaults to the built-in logic, which
 * relies upon the frame pointer register (EBP on 686).
 * The ZERO_USE_FRAME_HANDLERS environment variable needs to be
 * set to a non-zero value, otherwise frame handlers (even when
 * detected) are ignored. The ZERO_USE_FRAME_HANDLERS environment
 * variable is enabled by default on the x86_64.
 *
 * @note FrameHandler instances can be used for other purposes too,
 * such as presenting the user with the illusion of a stack frame
 * for inlined function calls
 */
DECLARE_ZDK_INTERFACE_(FrameHandler, Unknown2)
{
    DECLARE_UUID("c512cb1f-81a2-4595-8f2f-71b4190ff106")

    virtual void init(const Thread*) = 0;

    /**
     * @return true on success, false otherwise
     */
    virtual bool unwind_step(const Thread*,
                             const Frame*,
                             EnumCallback<Frame*>*) = 0;
};

#endif // FRAME_HANDLER_H__2DAA3AA7_BB39_4F5B_91D7_2E6947372B0D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
