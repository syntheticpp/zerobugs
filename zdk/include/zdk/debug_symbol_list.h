#ifndef DEBUG_SYMBOL_LIST_H__DF8B547C_AD47_4389_92FC_6199B6C8F7DC
#define DEBUG_SYMBOL_LIST_H__DF8B547C_AD47_4389_92FC_6199B6C8F7DC
//
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
#include <vector>

class DebugSymbol;

typedef std::vector<RefPtr<DebugSymbol> > DebugSymbolList;

#endif // DEBUG_SYMBOL_LIST_H__DF8B547C_AD47_4389_92FC_6199B6C8F7DC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
