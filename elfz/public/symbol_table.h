#ifndef SYMBOL_TABLE_H__F4E883D7_8CAB_4CAB_8A6A_B0D39E9F3AF3
#define SYMBOL_TABLE_H__F4E883D7_8CAB_4CAB_8A6A_B0D39E9F3AF3
//
// $Id: symbol_table.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "public/section.h"
#include "public/symbol_iterator.h"


namespace ELF
{
    class Symbol;


    class SymbolTable : public Section
    {
    public:
        typedef SymbolIterator<Symbol*, Symbol&> iterator;
        typedef SymbolIterator<const Symbol*, const Symbol&> const_iterator;

        friend class IterationTraits<SymbolTable>;

        SymbolTable(Elf*, Elf_Scn*);

        size_t size() const
        { return SymbolIteratorBase::size(*this); }

        /**
         * Get the next symbol table. If last table,
         * return a SymbolTable for which is_null()
         * yields true.
         */
        boost::shared_ptr<SymbolTable> next() const;

        const_iterator begin() const;
        const_iterator end() const;

    protected:
        /**
         * Traverse sections beginning with the given one
         * (inclusively) until a symbol table is found.
         */
        static boost::shared_ptr<SymbolTable>
            next_symbol_table(boost::shared_ptr<Section>);

        explicit SymbolTable(const Section&);
    };


    template<> struct IterationTraits<SymbolTable>
    {
        static boost::shared_ptr<SymbolTable> first(File&);

        static boost::shared_ptr<SymbolTable>
            next(File*, const SymbolTable& symtab)
        {
            return symtab.next();
        }
    };


    inline size_t SymbolIteratorBase::size(const Section& table)
    {
        const size_t symSize = (table.header().klass() == ELFCLASS32)
            ? sizeof (Elf32_Sym)
            : sizeof (Elf64_Sym);

        assert ((table.data().d_size % symSize) == 0);
        return table.data().d_size / symSize;
     }


     inline SymbolIteratorBase::SymbolIteratorBase(const Section& symTab, const char* data)
        : symTab_(symTab)
        , ptr_(data)
        , end_(data + symTab.data().d_size)
        , step_(0)
        , read_(NULL)
     {
        if (symTab.header().klass() == ELFCLASS32)
        {
            step_ = sizeof(Elf32_Sym);
            read_ = &SymbolIteratorBase::sym32;
        }
        else
        {
            step_ = sizeof(Elf64_Sym);
            read_ = &SymbolIteratorBase::sym64;
        }

     }

} // namespace ELF
#endif // SYMBOL_TABLE_H__F4E883D7_8CAB_4CAB_8A6A_B0D39E9F3AF3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
