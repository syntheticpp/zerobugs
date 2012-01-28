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
// Uses the elfz C++ wrapper library to read a core file.

#include <algorithm>
#include <exception>
#include <iomanip>
#include <iostream>

#include "public/binary.h"
#include "public/headers.h"
#include "public/note.h"
#include "public/program_header.h"
#include "public/section.h"
#include "public/symbol_table.h"

using namespace std;
using namespace ELF;


static void print_note(ostream& out, const Note& note)
{
    out << note.name();

    out << " descsz=" << hex << note.descsz();
    out << " (" << dec << note.descsz() << ")";

    out << " type=" << note.type() << endl;
}



static void print_core_file(ostream& out, const Binary& elf, const ElfHdr& hdr)
{
    out << hdr.phnum() << " program header(s)\n";

    const List<ProgramHeader>& phdrs = elf.program_headers();
    List<ProgramHeader>::const_iterator i = phdrs.begin();

    for (; i != phdrs.end(); ++i)
    {
        out << hex << i->offset() << dec;
        out << " type=" << i->type();
        out << " flags=" << i->flags();
        out << " vaddr=" << hex << i->vaddr() << dec;
        out << " filesz=" << i->filesz();
        out << " memsz=" << i->memsz() << endl;

        if (i->type() != PT_NOTE)
        {
            continue;
        }

        if (i->filesz())
        {
            vector<unsigned char> bytes(i->filesz());
            elf.readbuf(i->offset(), (char*)&bytes[0], i->filesz());

            unsigned char* data = &bytes[0];

            for (;;)
            {
                ELF::Note note(elf, data);

                assert(!note.is_null());

                print_note(out, note);

                data += note.total_rounded_size();

                const size_t size = distance(&bytes[0], data);

                if (size >= bytes.size())
                {
                    cout << "size=" << size << " bytes=" << bytes.size() << endl;
                    break;
                }
            }
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

            Binary::Header hdr = elf.header();
            if (hdr.type() == ET_CORE)
            {
                print_core_file(cout, elf, hdr);
            }
            else
            {
                cerr << *argv << " is not a core file\n";
            }
        }
        catch (const exception& e)
        {
            cerr << "exception: " << e.what() << endl;
        }
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
