#ifndef EVAL_SYM_H__E57D2DD2_0E6B_40C5_8E56_CA19EFF8423D
#define EVAL_SYM_H__E57D2DD2_0E6B_40C5_8E56_CA19EFF8423D
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

#include <string>
#include "zdk/debug_sym.h"
#include "zdk/variant.h"
#include "context.h"


RefPtr<Variant> eval_sym(
    Context&            context,
    const std::string&  name,
    RefPtr<DebugSymbol> symbol);

#endif // EVAL_SYM_H__E57D2DD2_0E6B_40C5_8E56_CA19EFF8423D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
