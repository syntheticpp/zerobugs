#ifndef STATIC_VECTOR_H__2DE30248_E257_498C_BF0B_694E6BC1015C
#define STATIC_VECTOR_H__2DE30248_E257_498C_BF0B_694E6BC1015C
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

#include <algorithm>
#include "nuts/assert.h"
#include "nuts/memory.h"

namespace nuts
{
    template<typename T, typename A = std::allocator<T> >
    class static_vector
    {
        A alloc_;
    public:
        typedef T value_type;
        typedef const T* const_iterator;

        explicit static_vector(size_t n = 0)
            : v_(0), capacity_(n), size_(0)
        {
            v_ = alloc_.allocate(n);
            //std::uninitialized_fill(v_, v_ + n, value_type());
        }
        static_vector(const static_vector& other, size_t offs = 0)
            : v_(0)
            , capacity_(other.capacity_)
            , size_(other.size())
        {
            v_ = alloc_.allocate(capacity_);
            if (size_ >= offs)
            {
                size_ -= offs;
                std::copy(other.begin() + offs, other.end(), v_);
            }
        }
        ~static_vector()
        {
            destroy(v_, v_ + size_);
            alloc_.dealocate(v_, capacity_);
        }
        size_t capacity() const
        {
            return capacity_;
        }
        size_t size() const
        {
            return size_;
        }
        bool empty() const
        {
            return size_ == 0;
        }
        void clear() { size_ = 0; }

        const_iterator begin() const { return v_; }

        const_iterator end() const { return v_ + size_; }

        void push_back(const value_type& v)
        {
            NUTS_ASSERT(size_ < capacity_);
            if (size_ < capacity_)
            {
                v_[size_++] = v;
            }
            else
            {
                //NUTS_EXHAUSTED_FIXED_MEM();
            }
        }
        const value_type& operator[](size_t i) const
        {
            NUTS_ASSERT(i < size_);
            return v_[i];
        }
        value_type& operator[](size_t i)
        {
            NUTS_ASSERT(i < size_);
            return v_[i];
        }
        const value_type& back() const
        {
            NUTS_ASSERT(size_);
            return v_[size_ - 1];
        }
        void swap(static_vector& other) throw()
        {
            std::swap(v_, other.v_);
            std::swap(capacity_, other.capacity_);
            std::swap(size_, other.size_);
        }
    private:
        static_vector(const static_vector&);
        static_vector& operator=(const static_vector&);

        T* v_;
        size_t capacity_;
        size_t size_;
    };
}
#endif // STATIC_VECTOR_H__2DE30248_E257_498C_BF0B_694E6BC1015C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
