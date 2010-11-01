#ifndef EVAL_INC_DEC_H__63B7E248_4B75_4FC0_A03B_B40CE2E53965
#define EVAL_INC_DEC_H__63B7E248_4B75_4FC0_A03B_B40CE2E53965
//
// $Id: eval_inc_dec.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/data_type.h"
#include "zdk/variant.h"
#include "context.h"

/**
 * Evaluate result of operators ++ and --
 * Used by both postfix and prefix expression evaluators
 */
RefPtr<Variant> eval_inc_dec(
    Context&        context,
    const DataType& exprType,
    const Variant&  lval,
    bool            increment); // decrement if false

#endif // EVAL_INC_DEC_H__63B7E248_4B75_4FC0_A03B_B40CE2E53965
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
