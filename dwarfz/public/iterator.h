#ifndef ITERATOR_H__362A8C39_5418_43F5_913C_640FBCA561E6
#define ITERATOR_H__362A8C39_5418_43F5_913C_640FBCA561E6
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

#include <iterator>
#include <memory>
#include "utils.h"
#include "interface.h"


namespace Dwarf
{
    template<typename T>
    struct IterationTraits
    {
        typedef std::shared_ptr<T> ptr_type;

        /**
         * Obtain the first element in the list
         */
        static ptr_type first(Dwarf_Debug dbg, Dwarf_Die die)
        {
            ptr_type p;

            if (Dwarf_Die child = Utils::first_child(dbg, die, T::TAG))
            {
                p = std::make_shared<T>(dbg, child);
            }
            return p;
        }

        /**
         * Get the sibling of same type for a given element
         */
        template<typename U>
        static void next(std::shared_ptr<U>& elem)
        {
            assert(elem);

            Dwarf_Debug dbg = elem->dbg();
            Dwarf_Die sibl = Utils::next_sibling(dbg, elem->die(), T::TAG);

            if (sibl)
            {
                elem = std::make_shared<T>(dbg, sibl);
            }
            else
            {
                elem.reset();
            }
        }
    };


    template<typename T, typename Ptr, typename Ref>
    CLASS Iterator : public std::shared_ptr<T>
    {
    public:
        typedef std::shared_ptr<T> base_type;

        typedef std::forward_iterator_tag iterator_category;
        typedef std::ptrdiff_t difference_type;

        typedef T value_type;
        typedef Ref reference;
        typedef Ptr pointer;

        /**
         * Avoid implicit conversion from const_iterator
         * to iterator -- they share the same base, and so
         * this function needs to be used to make an iterator
         * from a shared_ptr, and not the protected ctor below.
         */
        static Iterator create(const std::shared_ptr<T>& ptr)
        {
            return Iterator(ptr);
        }

    protected:
        explicit Iterator(const std::shared_ptr<T>& ptr)
            : base_type(ptr)
        {}

    public:
        typedef Iterator<T, T*, T&> iterator;

        /**
         * Implicit ctor to allow conversion from
         * iterator to const_iterator.
         */
        Iterator(const iterator& other) : base_type(other)
        {}

        reference operator*() const { return *this->get(); }

        pointer operator->() const { return this->get(); }

        Iterator& operator++()
        {
            increment();
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;

            increment();
            return tmp;
        }

    private:
        void increment()
        {
            if (this->get())
            {
                IterationTraits<T>::next(*this);
            }
        }
    };
}
#endif // ITERATOR_H__362A8C39_5418_43F5_913C_640FBCA561E6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
