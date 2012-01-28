#ifndef LIST_H__43298869_A527_497E_8FB2_454BCA7E13AE
#define LIST_H__43298869_A527_497E_8FB2_454BCA7E13AE
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

#include "interface.h"
#include "iterator.h"

namespace Dwarf
{
    template<typename T> struct Traits
    {
        typedef typename T::parent_type parent_type;
    };


    template<typename T> CLASS List
    {
        typedef T value_type;

        typedef boost::shared_ptr<T> ptr_type;
        typedef typename Traits<T>::parent_type parent_type;

    #ifdef __INTEL_COMPILER
        friend parent_type;
        //
        // T's parent can construct a List of T
        //
    #elif ((__GNUC__ >= 3) && (__GNUC_MINOR__ >= 4 || __GNUC_PATCHLEVEL__ >= 5)) \
        || (__GNUC__ >= 4)
        friend class Traits<T>::parent_type;

    #else
        friend typename Traits<T>::parent_type;
    #endif

        template<typename V> friend class List;

        /**
         * Obtain the first element in the list
         */
        ptr_type head() const
        {
            return IterationTraits<T>::first(dbg_, die_);
        }

    public:
        typedef Iterator<T, T*, T&> iterator;
        typedef Iterator<T, const T*, const T&> const_iterator;

        typedef size_t size_type;

        template<typename U>
        List(const List<U>& other)
            : dbg_(other.dbg_)
            , die_(other.die_)
            , end_(const_iterator::create(typename List<U>::ptr_type()))
        {
        }
        const_iterator begin() const
        {
            return const_iterator::create(head());
        }
        const_iterator end() const
        {
            return end_;
        }

        iterator begin() { return iterator::create(head()); }
        iterator end() { return iterator::create(ptr_type()); }

        bool empty() const { return !head(); }

        size_type size() const
        {
            return std::distance(begin(), end());
        }

    private:
        List(Dwarf_Debug dbg, Dwarf_Die parent)
            : dbg_(dbg)
            , die_(parent)
            , end_(const_iterator::create(ptr_type()))
        { }

    private:
        Dwarf_Debug dbg_;
        Dwarf_Die die_;
        const_iterator end_;
    };
}

#endif // LIST_H__43298869_A527_497E_8FB2_454BCA7E13AE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
