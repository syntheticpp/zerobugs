#ifndef FOBJECT_H__2322C2A2_9414_4CAE_9F92_F6538290ECE1
#define FOBJECT_H__2322C2A2_9414_4CAE_9F92_F6538290ECE1
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

#include "zdk/fallocator.h"


template<size_t K = 4>
struct ZDK_LOCAL Fobject
{
    void* operator new(size_t bytes)
    { return Fheap<K>::allocate(bytes); }

    void* operator new[](size_t bytes)
    { return Fheap<K>::allocate(bytes); }

    void operator delete(void* p, size_t bytes)
    { Fheap<K>::dealocate(p, bytes); }

    void operator delete[](void* p, size_t bytes)
    { Fheap<K>::dealocate(p, bytes); }

protected:
    Fobject() { };
    virtual ~Fobject() { };
};


#endif // FOBJECT_H__2322C2A2_9414_4CAE_9F92_F6538290ECE1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
