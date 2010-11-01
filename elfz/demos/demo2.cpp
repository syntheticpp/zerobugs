//
// $Id: demo2.cpp 711 2010-10-16 07:09:23Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <exception>
#include <iomanip>
#include <iostream>
#include <boost/cstdint.hpp>
#include "public/binary.h"
#include "public/link.h"

#include "public/section.h"
#include "public/symbol_table.h"

using namespace std;
using namespace ELF;


static void print_sections(ostream& out, const Binary& elf)
{
    out << "--- sections ---\n";

    const List<Section>& sections = elf.sections();
    List<Section>::const_iterator i = sections.begin();
    for (; i != sections.end(); ++i)
    {
        out << setw(2 * sizeof(long)) << setfill('0');
        out << hex << i->header().addr() << dec;
        out << ' ' << i->header().name() << endl;
/*
        if (strcmp(i->header().name(), ".dynamic") == 0)
        {
            const Elf_Data& d = i->data();
            const size_t count = d.d_size / sizeof(ElfW(Dyn));
            cout << "d_size=" << d.d_size << " count=" << count << endl;
            for ( size_t n = 0; n < count; ++n )
            {
                const ElfW(Dyn)* dyn = ( (ElfW(Dyn)*)d.d_buf ) + n;

                if (dyn->d_tag == DT_DEBUG)
                {
                    cout << "d_tag=0x" << hex << dyn->d_tag << endl;
                    cout << "d_ptr=0x" << dyn->d_un.d_ptr << endl;
                    cout << "section addr=" << i->header().addr() << dec << endl;
                    cout << "section offs=" << i->header().offset() << endl;
                }
            }
        }
 */
    }
}


static void print_symbols(ostream& out, const Binary& elf)
{
    const List<SymbolTable>& symtabs = elf.symbol_tables();
    List<SymbolTable>::const_iterator j = symtabs.begin();

    for (; j != symtabs.end(); ++j)
    {
        out << "--- " << j->header().name() << " ---\n";

        SymbolTable::const_iterator k = j->begin();
        for (; k != j->end(); ++k)
        {
            if ((k->type() != STT_FUNC && k->bind() != STB_GLOBAL)
              || k->bind() == STB_WEAK
            #if !__FreeBSD__ // todo: investigate
              || k->bind() == STB_NUM
            #endif
              )
            {
                continue;
            }

            out << setw(2 * sizeof(long)) << setfill('0');
            out << hex << (*k).value() << dec;
            out << ' ' << (*k).name() << endl;
        }
    }
}


int main(int argc, char* argv[])
{
    for (--argc, ++argv; argc; --argc, ++argv)
    {
        try
        {
            Binary elf(*argv);
            cout << "elf_kind=" << elf.kind() << endl;

            const List<Binary>& arch = elf.archive();
            List<Binary>::const_iterator i = arch.begin();

            for (; i != arch.end(); ++i)
            {
                if (i->kind() == ELF_K_ELF)
                {
                    print_sections(cout, *i);
                    print_symbols(cout, *i);
                }
            }
        }
        catch (const exception& e)
        {
            cerr << "exception: " << e.what() << endl;
        }
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
