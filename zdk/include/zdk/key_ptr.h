#ifndef KEY_PTR_H__DE438A42_93AF_43D8_B39E_5354D8A4BCB6
#define KEY_PTR_H__DE438A42_93AF_43D8_B39E_5354D8A4BCB6
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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

/**
 * Pair up a RefPtr with a key value (used to sort
 * and search by) so that locality of reference is
 * maintained.
 * Chasing down pointers is avoided when sorting
 * or searching by Key
 */
template<typename T, typename K>
class KeyPtr : public RefPtr<T>
{
    K k_;

public:
    KeyPtr(T* p, K k) : RefPtr<T>(p), k_(k) { }

    KeyPtr(const RefPtr<T>& p, K k) : RefPtr<T>(p), k_(k) { }

    K key() const { return k_; }
};
#endif // KEY_PTR_H__DE438A42_93AF_43D8_B39E_5354D8A4BCB6
