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
//
// This program uses elfz C++ wrapper to get a stacktrace from a core file.

#include <exception>
#include <iomanip>
#include <iostream>

#include "elfz/public/core_file.h"
#include "symbolz/public/symbol_table_events.h"

/* should not include private headers, really, this is
   here just for test/demo purposes. */
#include "symbolz/private/symbol_map_impl.h"

using namespace std;


class Events : public SymbolTableEvents
{
    void notify(BreakPoint*, const SymbolTable*)
    {
    }

    void notify(BreakPoint*, addr_t)
    {
    }

    addr_t next_line(const RefPtr<Symbol>&) const { return 0; }
};


static void print(ostream& out, const Symbol& sym)
{
    out << sym.file();

    if (sym.line())
    {
        out << ':' << sym.line();
    }
    else
    {
        out << ' '  << sym.demangled_name(false);
        out << "+0x" << hex << sym.offset() << dec;
    }
}


void print_trace(const ELF::CoreFile& core,
                 //const SymbolMap& symbols,
                 const elf_prstatus& stat)
{
    cout << "##### tid=" << stat.pr_pid
#if !__FreeBSD__
         << " ppid=" << stat.pr_ppid
#endif
         ;
    cout << " signal=" << stat.pr_cursig << endl;
    const user_regs_struct& regs =
        reinterpret_cast<const user_regs_struct&>(stat.pr_reg);
#if __x86_64__
    long esp = regs.rsp;
    long eip = regs.rip;
    long ebp = regs.rbp;
#else
    long esp = regs.esp;
    long eip = regs.eip;
    long ebp = regs.ebp;
#endif
    for (;;)
    {
        cout << hex << setw(8) << /*esp << ' ' << */ eip << dec << ' ';

        if (eip != (long)0xFFFFE002)
        {
            /*
            RefPtr<Symbol> sym = symbols.lookup_symbol(eip);
            if (sym.get())
            {
                print(cout, *sym);
            }
            */
            cout << (void*)eip;
        }
        cout << endl;

        // if (ebp == 0) break;

        esp = ebp + sizeof(long);
#if __x86_64__
        if ((unsigned long)esp < regs.rsp)
        {
            break;
        }
#else
        if ((unsigned long)esp < (unsigned long)regs.esp)
        {
            break;
        }
#endif
        core.readval(esp, eip);
        core.readval(ebp, ebp);
    }
}


int main(int argc, char* argv[])
{
    if (argc < 1)
    {
        cerr << "usage: " << *argv << " corefile\n";
        return 1;
    }

    try
    {
        cout << "reading corefile\n";

        ELF::CoreFile core(argv[1]);
        cout << "reading symbols...\n";

        Events events;

        // fixme
        //SymbolMapImpl symbolsMap(core, events);
        //SymbolMap& symbols = symbolsMap;

        //cout << "symbols ok\n";

        ELF::CoreFile::const_prstatus_iterator i = core.prstatus_begin();
        for (; i != core.prstatus_end(); ++i)
        {
            print_trace(core, i->second);
        }
    }
    catch (const exception& e)
    {
        cerr << "exception: " << e.what() << endl;
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
