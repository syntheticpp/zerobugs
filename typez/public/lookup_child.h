#ifndef LOOKUP_CHILD_H__E747C894_694C_44D9_BF3D_F806E415122F
#define LOOKUP_CHILD_H__E747C894_694C_44D9_BF3D_F806E415122F
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

#include "zdk/debug_sym.h"

/**
 * Lookup a named field in a class, struct, or union;
 * recursively descends into base classes. Return the child
 * symbol if found, or null otherwise.
 */
RefPtr<DebugSymbol> lookup_child(DebugSymbol&, const char* name);

#endif // LOOKUP_CHILD_H__E747C894_694C_44D9_BF3D_F806E415122F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
