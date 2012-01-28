//
// $Id$
//
// Implementations for SymbolTable and SymbolTableList
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include "public/binary.h"
#include "public/symbol_table.h"


using namespace std;
using namespace ELF;



SymbolTable::SymbolTable(Elf* elf, Elf_Scn* scn) : Section(elf, scn)
{
    assert(is_null() || is_symbol_table(*this));
}


SymbolTable::SymbolTable(const Section& sec) : Section(sec)
{
    assert(is_null() || is_symbol_table(*this));
}


/**
 * Traverse sections beginning with the given one
 * (inclusively) until a symbol table is found
 */
boost::shared_ptr<SymbolTable>
SymbolTable::next_symbol_table(boost::shared_ptr<Section> s)
{
    for (; s && !is_symbol_table(*s); s = s->next())
    { }

    boost::shared_ptr<SymbolTable> symTbl;
    if (s)
    {
        boost::shared_ptr<SymbolTable> p(new SymbolTable(*s));
        symTbl = p;
    }
    return symTbl;
}


boost::shared_ptr<SymbolTable> SymbolTable::next() const
{
    return next_symbol_table(Section::next());
}


SymbolTable::const_iterator SymbolTable::begin() const
{
    assert(!is_null());

    char* ptr = reinterpret_cast<char*>(data().d_buf);

    assert(header().elf());
    return const_iterator(*this, ptr);
}


SymbolTable::const_iterator SymbolTable::end() const
{
//
// todo: cache end()
//
    assert(!is_null());

    const Elf_Data& d = data();
    char* ptr = static_cast<char*>(d.d_buf) + d.d_size;
    return const_iterator(*this, ptr);
}


boost::shared_ptr<SymbolTable>
IterationTraits<SymbolTable>::first(File& file)
{
    boost::shared_ptr<Section> sec =
        IterationTraits<Section>::first(file);

    return SymbolTable::next_symbol_table(sec);
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
