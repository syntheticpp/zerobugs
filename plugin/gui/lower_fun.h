#ifndef LOWER_FUN_H__CF53544F_AC49_45D0_939C_724BC662D44C
#define LOWER_FUN_H__CF53544F_AC49_45D0_939C_724BC662D44C
//
// $Id: lower_fun.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zero.h"

/// Find the function with the lower address defined
/// in the translation unit whose file name is `srcPath'
/// @return a null pointer if no function found

RefPtr<Symbol> lower_fun(Debugger&, const char* srcPath);


/// Find the function with the lower address defined
/// in the translation unit that contains `addr'
/// @return zero if no function found

// Platform::addr_t lower_fun(Debugger&, addr_t addr);

Platform::addr_t lower_fun(Debugger&, const Symbol&);

#endif // LOWER_FUN_H__CF53544F_AC49_45D0_939C_724BC662D44C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
