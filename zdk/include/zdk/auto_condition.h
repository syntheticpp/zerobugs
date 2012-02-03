#ifndef AUTO_CONDITION_H__B5722D5A_0682_453F_BE11_30A1499AB051
#define AUTO_CONDITION_H__B5722D5A_0682_453F_BE11_30A1499AB051
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
#include "zdk/mutex.h"

/**
 * Helper class, for automatically signaling
 * a condition when a scope is exited.
 */
class AutoCondition
{
    //non-copyable, non-assignable
    AutoCondition(const AutoCondition&);
    AutoCondition& operator=(const AutoCondition&);

    Condition& cond_;

public:
    explicit AutoCondition(Condition& cond) : cond_(cond) {}
    ~AutoCondition() { cond_.broadcast(); }
};

#endif // AUTO_CONDITION_H__B5722D5A_0682_453F_BE11_30A1499AB051
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

