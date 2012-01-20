#ifndef SARRAY_H__DFC32E30_214A_4518_B337_DB1D4FBF8FB8
#define SARRAY_H__DFC32E30_214A_4518_B337_DB1D4FBF8FB8
//
// $Id: sarray.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/export.h"
#include "zdk/stdexcept.h"
#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <string>
#include <vector>


/**
 * A vector of strings, with support for conversion to
 * char* const* and char**, so that it can be used with
 * C functions from the exec family.
 */
class ZDK_LOCAL SArray
{
    struct StrPtr
    {
        explicit StrPtr(const char* s = NULL) { s_.const_str = s; }

        StrPtr& operator=(const char* const_str)
        { s_.const_str = const_str; return *this; }

        union { const char* const_str; char* str; } s_;
    };

public:
    typedef std::deque<std::string> container_type;

    SArray() : isDirty_(false) { }

    template<typename Iter>
    SArray(Iter first, Iter last) : isDirty_(false)
    {
        assign(first, last);
        assert(isDirty_);
    }
    explicit SArray(const char* const* p) : isDirty_(true)
    {
        for (; p && *p; ++p)
        {
            strings_.push_back(*p);
        }
    }
    virtual ~SArray()
    {
#if DEBUG
        std::fill(ptrs_.begin(), ptrs_.end(), StrPtr((char*)0xbadcaca));
#endif
    }
    const char* operator[](size_t n) const
    {
        assert (n < strings_.size());
        if (n >= strings_.size())
        {
            throw std::out_of_range("array index is out of range");
        }
        update_cstrings();
        return strings_[n].c_str();
    }
    char* const* cstrings() const
    {
        update_cstrings();
        assert(!isDirty_);
        return ptrs_.size() ? (&ptrs_[0].s_.str) : NULL;
    }
    char** get()
    {
        update_cstrings();
        assert(!isDirty_);
        return ptrs_.size() ? (&ptrs_[0].s_.str) : NULL;
    }
    SArray(const SArray& other)
        : strings_(other.strings_)
        , isDirty_(true)
    {
    }
    SArray& operator=(const SArray& other)
    {
        SArray tmp(other);
        tmp.swap(*this);
        return *this;
    }
    void clear()
    {
        strings_.clear();
        ptrs_.clear();
        isDirty_ = false;
    }
    void swap(SArray& other)
    {
        strings_.swap(other.strings_);
        ptrs_.swap(other.ptrs_);
        std::swap(isDirty_, other.isDirty_);
    }
    const container_type& strings() const
    {
        return strings_;
    }
    void push_front(const std::string& s)
    {
        strings_.push_front(s);
        isDirty_ = true;
    }
    void push_back(const std::string& s)
    {
        strings_.push_back(s);
        isDirty_ = true;
    }
    void push_back(const char* s)
    {
        std::string tmp;
        if (s)
        {
            tmp.assign(s);
        }
        push_back(tmp);
    }
    void pop_front() { strings_.pop_front(); isDirty_ = true; }
    void pop_back() { strings_.pop_back(); isDirty_ = true; }

    size_t size() const { return strings_.size(); }

    bool empty() const { return strings_.empty(); }

    template<typename Iter>
    void assign(Iter first, Iter last)
    {
        strings_.assign(first, last);
        isDirty_ = true;
    }
    template<typename T>
    T for_each(T fn)
    {
        return std::for_each(strings_.begin(), strings_.end(), fn);
    }

private:
    void update_cstrings() const
    {
        if (isDirty_)
        {
            ptrs_.resize(strings_.size() + 1);

            std::transform(
                strings_.begin(), strings_.end(),
                ptrs_.begin(),
                std::mem_fun_ref(&std::string::c_str));

            isDirty_ = false;
        }
    }

private:
    container_type strings_;
    mutable std::vector<StrPtr> ptrs_;
    mutable bool isDirty_;
};


#endif // SARRAY_H__DFC32E30_214A_4518_B337_DB1D4FBF8FB8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
