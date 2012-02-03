#ifndef SYMBOL_ITERATOR_H__A34F9CC8_767B_4936_92D6_AFE81C2D6BFF
#define SYMBOL_ITERATOR_H__A34F9CC8_767B_4936_92D6_AFE81C2D6BFF
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

#include <cassert>
#include <iterator>
#include "public/section.h"
#include "public/symbol.h"


namespace ELF
{
    class SymbolIteratorBase
    {
    public:
        friend bool operator==(const SymbolIteratorBase&,
                               const SymbolIteratorBase&);

        static size_t size(const Section& table);

    protected:
        SymbolIteratorBase(const Section& symTab, const char* data);

        Symbol& deref() { return sym_; }
        const Symbol& deref() const { return sym_; }

        void increment()
        {
            if (ptr_ < end_)
            {
                ptr_ += step_;
            }
            else
            {
                assert(ptr_ == end_);
            }
            if (ptr_ < end_)
            {
                (this->*read_)();
            }
        }

    private:
        void sym32()
        {
            assert(ptr_ < end_);
            const SectionHdr& hdr = symTab_.header();
            const char* name = elf_strptr(hdr.elf(), hdr.link(), sym32_->st_name);
            sym_ = Symbol(name, sym32_->st_value, sym32_->st_size, sym32_->st_info);
        }
        void sym64()
        {
            assert(ptr_ < end_);
            const SectionHdr& hdr = symTab_.header();
            const char* name = elf_strptr(hdr.elf(), hdr.link(), sym64_->st_name);
            sym_ = Symbol(name, sym64_->st_value, sym64_->st_size, sym64_->st_info);
        }

    private:
        const Section& symTab_;

        union
        {
            const char* ptr_;
            const Elf32_Sym* sym32_;
            const Elf64_Sym* sym64_;
        };
        const char* end_;
        size_t step_;
        Symbol sym_;
        void (SymbolIteratorBase::*read_)();
    };


    bool inline operator==(const SymbolIteratorBase& lhs,
                           const SymbolIteratorBase& rhs)
    {
        return (lhs.ptr_ == rhs.ptr_) && (&lhs.symTab_ == &rhs.symTab_);
    };

    bool inline operator!=(const SymbolIteratorBase& lhs,
                           const SymbolIteratorBase& rhs)
    {
        return !(lhs == rhs);
    };


    template<typename Pointer, typename Reference>
    class SymbolIterator
        : public SymbolIteratorBase
        , public std::iterator<
            std::forward_iterator_tag,
            Symbol,
            ptrdiff_t,
            Pointer,
            Reference>
    {
        friend class SymbolTable;

    protected:
        /**
         * ctor is protected: only the symbol table can construct
         */
        SymbolIterator(const Section& symtab, char* ptr)
            : SymbolIteratorBase(symtab, ptr)
        { }

    public:
        typedef SymbolIterator<Symbol*, Symbol&> Iterator;

        /**
         * Implicit ctor to allow conversion from
         * iterator to const_iterator.
         */
        SymbolIterator(const Iterator& other)
            : SymbolIteratorBase(other)
        { }

        Reference operator*() const { return this->deref(); }

        Pointer operator->() const { return &deref(); }

        SymbolIterator& operator++()
        {
            increment();
            return *this;
        }

       /* not used
        SymbolIterator& operator--()
        {
            decrement();
            return *this;
        }

        SymbolIterator operator++(int)
        {
            SymbolIterator tmp(*this);
            increment();

            return tmp;
        }

        SymbolIterator operator--(int)
        {
            SymbolIterator tmp(*this);
            decrement();

            return tmp;
        }
      */
    };
}
// Copyright (c) 2004 Cristian L. Vlasceanu


#endif // SYMBOL_ITERATOR_H__A34F9CC8_767B_4936_92D6_AFE81C2D6BFF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
