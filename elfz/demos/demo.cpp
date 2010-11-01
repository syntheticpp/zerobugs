//
// $Id: demo.cpp 711 2010-10-16 07:09:23Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <exception>
#include <iostream>

#include "public/binary.h"
#include "public/section.h"
#include "public/symbol_table.h"

using namespace std;
using namespace ELF;


int main(int argc, char* argv[])
{
    for (--argc, ++argv; argc; --argc, ++argv)
    {
        try
        {
            Binary elf(*argv);
            elf.check_format();
            cout << "--- sections ---\n";

            const List<Section>& sections = elf.sections();

            List<Section>::const_iterator i = sections.begin();
            for (; i != sections.end(); ++i)
            {
                cout << hex << i->header().addr() << dec;
                cout << ' ' << i->header().type();
                cout << ' ' << i->header().link();
                cout << ' ' << i->header().name() << endl;
            }

            cout << "--- symbol tables ---\n";

            const List<SymbolTable>& symtabs = elf.symbol_tables();
            List<SymbolTable>::const_iterator j = symtabs.begin();

            for (; j != symtabs.end(); ++j)
            {
                cout << "--- " << j->header().name() << " ---\n";

                SymbolTable::const_iterator k = j->begin();
                for (; k != j->end(); ++k)
                {
                    if ((k->type() != STT_FUNC && k->bind() != STB_GLOBAL)
                      || k->bind() == STB_WEAK
                #if !__FreeBSD__
                      || k->bind() == STB_NUM
                #endif
                      )
                    {
                        continue;
                    }

                    cout << hex << (*k).value() << dec
                         << ' ' << (*k).name() << endl;
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
