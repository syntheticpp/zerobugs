// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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

#include "zdk/thread_util.h"
#include "jump.h"


bool is_plt_jump(Thread& thread, const Symbol*, addr_t pc)
{
    word_t w = 0;
    size_t wordsRead = 0;

    thread.read_code(pc, &w, 1, &wordsRead);

    const bool result = (w == 0x3d601001); // lis r11, 4097

    return result;
}
