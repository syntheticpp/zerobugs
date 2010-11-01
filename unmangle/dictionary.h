#ifndef DICTIONARY_H__34E1EF57_E70B_447B_BC07_B95D024369FB
#define DICTIONARY_H__34E1EF57_E70B_447B_BC07_B95D024369FB
//
// $Id: dictionary.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <vector>
#include "nuts/static_vector.h"
#include <iostream>

#ifdef USE_STD_CONTAINERS
template<typename T>
struct DictBase
{
    typedef T value_type;
    typedef std::vector<T> container_type;

protected:
    explicit DictBase(size_t = 0) { }

    DictBase(const container_type& c, size_t pos)
        : elems_(c.begin() + pos, c.end()) { }

    void swap(DictBase& other) throw() { elems_.swap(other.elems_); }

    container_type elems_;
};
#else
template<typename T>
struct DictBase
{
    typedef T value_type;
    typedef nuts::static_vector<T, nuts::alloc<T> > container_type;

protected:
    explicit DictBase(size_t n = 0) : elems_(n) { }

    DictBase(const container_type& c, size_t pos) : elems_(c, pos) { }

    void swap(DictBase& other) throw() { elems_.swap(other.elems_); }

    container_type elems_;
};
#endif // USE_STD_CONTAINERS

template<typename C>
struct DefaultPolicy
{
    typedef C container_type;
    typedef typename C::value_type value_type;

    static void insert(container_type& c, const value_type& v, bool = false)
    {
#ifdef DEBUG_DICT
        std::clog << c.size() << ": " << v << std::endl;
#endif
        c.push_back(v);
    }
    // place holders, for policies that need to store state
    void swap(DefaultPolicy&) throw() { }

    void clear() { }
};

template<typename C>
struct SubstPolicy
{
    typedef C container_type;
    typedef typename C::value_type value_type;

    static void
    insert(container_type& c, const value_type& v, bool override = false)
    {
        if (override || c.empty() || (c.back() != v))
        {
#ifdef DEBUG_DICT
            std::clog << c.size() << ": " << v << std::endl;
#endif
            NUTS_ASSERT(v[0] != ',');

            if (v[0] != ':')
            {
                c.push_back(v);
            }
        }
    }
    void swap(SubstPolicy&) throw() { }

    void clear() { }
};

template<typename T, typename P = DefaultPolicy<typename DictBase<T>::container_type> >
class Dictionary : public DictBase<T>
{
public:
    typedef DictBase<T> base_type;
    typedef P policy_type;
    typedef T value_type;

    explicit Dictionary(size_t n = 0) : base_type(n) { }

    Dictionary(const Dictionary& other, size_t pos = 0)
        : base_type(other.elems_, pos)
    { }

    void insert(const T& v, bool override = false)
    {
        if (!v.empty())
        {
            policy_.insert(base_type::elems_, v, override);
        }
    }
    template<typename U>
    bool output_elem(U& output, size_t i) const
    {
        if (i < base_type::elems_.size())
        {
            const T& v = base_type::elems_[i];
#ifdef DEBUG_DICT
            std::clog << "[" << i << "]-->" << v << std::endl;
#endif
            output.append(v.data(), v.size());
            return true;
        }
        return false;
    }
    const value_type& operator[](size_t i) const
    {
        NUTS_ASSERT(i < base_type::elems_.size());
        return base_type::elems_[i];
    }
    Dictionary& swap(Dictionary& other) throw()
    {
        base_type::swap(other);
        policy_.swap(other.policy_);
        return *this;
    }
    size_t size() const { return base_type::elems_.size(); }

    bool empty() const { return base_type::elems_.empty(); }
/*
    void reserve(size_t size)
    {
#if defined(USE_STD_CONTAINERS)
        base_type::elems_.reserve(size);
#endif
    }
*/
    void resize(size_t size) { base_type::elems_.resize(size); }

    void clear()
    {
        base_type::elems_.clear();
        policy_.clear();
    }

private:
    policy_type policy_;
};
#endif // DICTIONARY_H__34E1EF57_E70B_447B_BC07_B95D024369FB
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
