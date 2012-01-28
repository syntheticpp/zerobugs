#ifndef FIXED_STACK_H__B30AF895_5099_43C2_8B75_E79B69655995
#define FIXED_STACK_H__B30AF895_5099_43C2_8B75_E79B69655995
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

#include "nuts/assert.h"

namespace nuts
{
    template<typename T, size_t N>
    class fixed_stack
    {
    public:
        typedef T value_type;
        typedef T* iterator;

        fixed_stack() : p_(0) { }

        ~fixed_stack() { }

        bool empty() const { return (p_ == 0); }

        size_t size() const { return p_; }

        size_t max_size() const { return N; }

        const value_type& top() const
        {
            NUTS_ASSERT(p_);
            return elem_at(p_ - 1);
        }
        value_type& top()
        {
            NUTS_ASSERT(p_);
            return elem_at(p_ - 1);
        }
        void pop()
        {
            NUTS_ASSERT(p_);
            if (p_) --p_;
        }
        void push(const value_type& v)
        {
            if (p_ < N)
            {
                elem_at(p_++) = v;
            }
            else
            {
                NUTS_EXHAUSTED_FIXED_MEM();
            }
        }
        void push(value_type& v)
        {
            if (p_ < N)
            {
                elem_at(p_++) = v;
            }
            else
            {
                NUTS_EXHAUSTED_FIXED_MEM();
            }
        }
   /*
        void push_back()
        {
            if (p_ < N)
                new (&elem_at(p_++)) T();
            else
                NUTS_EXHAUSTED_FIXED_MEM();
        }
    */
        // minimal compatibility with sequence containers
        // a standard stack does not have these
        void push_back(const value_type& v) { push(v); }

        void pop_back() { pop(); }

        value_type& back() { NUTS_ASSERT(p_); return elem_at(p_ - 1); }

        // const value_type& back() const { return top(); }

        iterator begin() { return &elem_at(0); }

        iterator end() { return &elem_at(p_); }

    private:
        value_type& elem_at(size_t i) { return elem_[i]; }
        value_type elem_[N];

        size_t p_;
    };
}
#endif // FIXED_STACK_H__B30AF895_5099_43C2_8B75_E79B69655995
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
