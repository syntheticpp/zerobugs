#ifndef COLLECT_SYMBOL_H__AFD238CE_6D05_471B_B284_CD855D8B1F26
#define COLLECT_SYMBOL_H__AFD238CE_6D05_471B_B284_CD855D8B1F26
//
// $Id: collect_symbol.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

class Method;
class TypeSystem;

void
collect_symbol( TypeSystem&,
                RefPtr<DebugSymbol>&,
                DebugSymbol&,
                Method* = NULL);

#endif // COLLECT_SYMBOL_H__AFD238CE_6D05_471B_B284_CD855D8B1F26
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
