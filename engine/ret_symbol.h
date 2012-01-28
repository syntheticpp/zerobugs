#ifndef RET_SYMBOL_H__EEF9E8FC_C213_4E6F_87E8_CC91A56A4289
#define RET_SYMBOL_H__EEF9E8FC_C213_4E6F_87E8_CC91A56A4289
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


struct Symbol;
struct Thread;
struct DebugSymbol;

/**
 * @return the debug symbol that models the result of function pFun (or NULL)
 */
RefPtr<DebugSymbol> ret_symbol(Thread*, RefPtr<Symbol> pFun);

#endif // RET_SYMBOL_H__EEF9E8FC_C213_4E6F_87E8_CC91A56A4289
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
