#ifndef FALLOCATOR_H__BBCCA53D_3651_4E70_AD19_C046CEEFE19D
#define FALLOCATOR_H__BBCCA53D_3651_4E70_AD19_C046CEEFE19D
//
// $Id$
//
// Adapt Falloc for use with STL containers
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/falloc.h"
#include "zdk/fheap.h"


template<typename T, size_t K> class Fallocator;


/**
 * Fallocator<void> specialization.
 */
template<size_t K>
class ZDK_LOCAL Fallocator<void, K>
{
public:
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef void*       pointer;
    typedef const void* const_pointer;
    typedef void        value_type;

    template<typename U>
    struct rebind { typedef Fallocator<U, K> other; };
};


template<typename T, size_t K = 4>
class ZDK_LOCAL Fallocator : public Fheap<(K * 1024)>
{
public:
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef T&          reference;
    typedef const T&    const_reference;
    typedef T           value_type;

    template<typename U>
    struct rebind { typedef Fallocator<U, K> other; };

    Fallocator() throw() { }

    Fallocator(const Fallocator&) throw() { }

    template<typename U>
    Fallocator(const Fallocator<U, K>&) throw() { }

    ~Fallocator() throw() { }

    pointer allocate(size_type n, const void* = 0)
    {
        const size_t size = n * sizeof(T);
        void* p = this->falloc.allocate(size);
        if (!p)
        {
            throw std::bad_alloc();
        }
        return reinterpret_cast<pointer>(p);
    }

    void deallocate(pointer p, size_type n)
    { this->falloc.dealocate(p, n * sizeof(T)); }

    void construct(pointer p, const T& val)
    { ::new(p) T(val); }

    void destroy(pointer p) { p->~T(); }
};


template<size_t K> Falloc<K * 1024> Fheap<K>::falloc;


template<typename T1, typename T2, size_t K>
inline bool ZDK_LOCAL
operator==(const Fallocator<T1, K>&, const Fallocator<T2, K>&)
{ return true; }

template<typename T1, typename T2, size_t K>
inline bool ZDK_LOCAL
operator!=(const Fallocator<T1, K>&, const Fallocator<T2, K>&)
{ return false; }

#endif // FALLOCATOR_H__BBCCA53D_3651_4E70_AD19_C046CEEFE19D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
