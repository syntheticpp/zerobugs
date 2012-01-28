#ifndef GET_POINTER_H__21D6748B_FBA5_4D87_ACE6_54C15D964434
#define GET_POINTER_H__21D6748B_FBA5_4D87_ACE6_54C15D964434
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

#include <memory>
#include "zdk/export.h"
#include "zdk/weak_ptr.h"


template<class P> inline P ZDK_LOCAL get_pointer(P p) { return p; }


template<class T>
inline ZDK_LOCAL T* get_pointer(const RefPtr<T>& p)
{
    return p.get();
}


template<class T>
inline ZDK_LOCAL T* get_pointer(const WeakPtr<T>& p)
{
    return p.ref_ptr().get();
}


/*
template<template<typename T> class P, typename T>
inline T* get_pointer(const P<T>& p)
{
    return p.get();
}
*/

template<typename T>
inline ZDK_LOCAL T* get_pointer(const std::auto_ptr<T>& p)
{
    return p.get();
}
#endif // GET_POINTER_H__21D6748B_FBA5_4D87_ACE6_54C15D964434
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
