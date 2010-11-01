#ifndef ITERATOR_H__5139B8E5_5DED_43B5_B805_895A5409BA44
#define ITERATOR_H__5139B8E5_5DED_43B5_B805_895A5409BA44
//
// $Id: iterator.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/ref_ptr.h"
#include <map>


template<typename T>
struct DerefTraits
{
    static T& project(T& v) { return v; }
};


template<typename K, typename V>
struct DerefTraits<std::pair<K, V> >
{
    static V& project(std::pair<K, V>& p) { return p.second; }
};



template<typename T, typename U, typename C>
class RefCountedIterator
{
public:
    typedef typename C::iterator iterator;

    typedef RefPtr<T> value_type;

    typedef std::bidirectional_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;
    typedef value_type* pointer;
    typedef value_type& reference;


    ~RefCountedIterator() { owner_->decrement_iter_count(); }

    RefCountedIterator(U& owner, iterator iter)
      : owner_(&owner), iter_(iter)
    {
        owner_->increment_iter_count();
    }

    RefCountedIterator(const RefCountedIterator& other)
        : owner_(other.owner_)
        , iter_(other.iter_)
    {
        owner_->increment_iter_count();
    }

    //pre-increment & pre-decrement operators;
    //post-increment and post-decrement forms not supported
    RefCountedIterator& operator++() { ++iter_; return *this; }
    RefCountedIterator& operator--() { --iter_; return *this; }

    reference operator*()
    {
        return DerefTraits<typename C::value_type>::project(*iter_);
    }

    pointer operator->()
    {
        return &DerefTraits<typename C::value_type>::project(*iter_);
    }

    bool operator==(const RefCountedIterator& other) const
    { return iter_ == other.iter_; }

    bool operator!=(const RefCountedIterator& that) const
    { return !(*this == that); }

    iterator base_iterator() { return iter_; }

private:
    RefCountedIterator& operator=(const RefCountedIterator&);

    U* owner_;
    iterator iter_;
};

#endif // ITERATOR_H__5139B8E5_5DED_43B5_B805_895A5409BA44
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
