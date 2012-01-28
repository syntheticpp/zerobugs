#ifndef MEMORY_H__9C916E46_FA85_448B_B7A0_4BCB0CBA18C3
#define MEMORY_H__9C916E46_FA85_448B_B7A0_4BCB0CBA18C3
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
#include <new>
#include <boost/checked_delete.hpp>


namespace nuts
{
    struct default_alloc
    {
        static void* allocate(size_t n)
        {
            void* p = ::malloc(n);
            if (!p)
            {
                throw std::bad_alloc();
            }
            return p;
        }
        static void dealocate(void* p /*, size_t size */)
        {
            ::free(p);
        }
        static void* reallocate(void* p, size_t size)
        {
            return ::realloc(p, size);
        }
    };
    template<typename T>
    struct alloc
    {
        static T* allocate(size_t n)
        {
            void* p = calloc(n, sizeof(T));
            if (!p)
            {
                throw std::bad_alloc();
            }
            return static_cast<T*>(p);
        }
        static void dealocate(T* p, size_t /* size */)
        { free(p); }
    };

    template<typename T> void destroy(T* first, T* last)
    {
        for (; first != last; ++first) { first->~T(); }
    }
}
#endif // MEMORY_H__9C916E46_FA85_448B_B7A0_4BCB0CBA18C3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
