#ifndef ITERATOR_H__FC9D4F5D_42CB_4BDE_9FB1_6DF4847A4D19
#define ITERATOR_H__FC9D4F5D_42CB_4BDE_9FB1_6DF4847A4D19
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
#include <iterator>
#include <boost/shared_ptr.hpp>


namespace ELF
{
    class File;

    template<typename T> struct IterationTraits {};

    /**
     * Generic iterator -- for traversing ELF sections,
     * symbol tables, archives, etc.
     */
    template<typename T,
             typename Pointer,
             typename Reference,
             typename Traits = IterationTraits<T> >
    class Iterator : public std::iterator<
            std::forward_iterator_tag,
            T,
            ptrdiff_t,
            Pointer,
            Reference>
    {
    public:
        typedef Iterator<T, T*, T&, Traits> iterator;

        template<typename U,
                 typename P,
                 typename R,
                 typename X> friend class Iterator;

        Iterator() : elf_(0)
        {
        }

        explicit Iterator(File& elf)
            : elf_(&elf)
            , ptr_(Traits::first(elf))
        {
        }

        /**
         * Allow conversion from iterator to const_iterator
         */
        Iterator(const iterator& other)
            : elf_(other.elf_), ptr_(other.ptr_)
        {
        }

        Reference operator*() const
        {
            assert(ptr_.get());
            return Reference(*ptr_);
        }

        Pointer operator->() const { return ptr_.get(); }

        Iterator& operator++()
        {
            increment();
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp(*this);
            increment();
            return tmp;
        }
        Iterator& operator=(Iterator other)
        {
            this->swap(other);
            return *this;
        }

        void increment()
        {
            if (ptr_)
            {
                ptr_ = Traits::next(elf_, *ptr_);
            }
            assert(!ptr_ || !ptr_->is_null());
        }

        void swap(Iterator& other) throw()
        {
            std::swap(elf_, other.elf_);
            ptr_.swap(other.ptr_);
        }

        template<typename P, typename R>
        bool operator==(const Iterator<T, P, R>& other) const
        {
            return ptr_.get() == other.ptr_.get();
        }

        template<typename P, typename R>
        bool operator!=(const Iterator<T, P, R>& other) const
        {
            return (ptr_.get() != other.ptr_.get());
        }
        
        template<typename P, typename R>
        bool operator<(const Iterator<T, P, R>& other) const
        {
            return (ptr_.get() < other.ptr_.get());
        }

    private:
        File* elf_;
        boost::shared_ptr<T> ptr_;
    };
} // namespace ELF


#endif // ITERATOR_H__FC9D4F5D_42CB_4BDE_9FB1_6DF4847A4D19
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
