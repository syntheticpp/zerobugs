#ifndef JUMP_H__0E3E5AFA_E70D_40CB_BAA8_2536583E6219
#define JUMP_H__0E3E5AFA_E70D_40CB_BAA8_2536583E6219
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
//
#include "zdk/platform.h"
#include "zdk/zerofwd.h"

using Platform::addr_t;


/**
 * Is the jump into the PLT?
 */
bool is_plt_jump(Thread& thread, const Symbol* sym, addr_t pc);

#endif // JUMP_H__0E3E5AFA_E70D_40CB_BAA8_2536583E6219
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
