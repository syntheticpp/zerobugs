#ifndef LOOKUP_METHODS_H__3BF34476_C78C_4E32_ACE9_4994A6575B8E
#define LOOKUP_METHODS_H__3BF34476_C78C_4E32_ACE9_4994A6575B8E
//
// $Id: lookup_methods.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <string>
#include "zdk/ref_ptr.h"

class ClassType;
class Context;
class DebugSymbol;

RefPtr<DebugSymbol>
lookup_methods(Context&, const ClassType&, const std::string&, addr_t = 0);

#endif // LOOKUP_METHODS_H__3BF34476_C78C_4E32_ACE9_4994A6575B8E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
