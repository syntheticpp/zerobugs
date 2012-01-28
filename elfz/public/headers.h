#ifndef HEADERS_H__48AF820C_8F74_453F_B098_F35A42BD9232
#define HEADERS_H__48AF820C_8F74_453F_B098_F35A42BD9232
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
// Most used headers: ElfHdr, SectionHdr -- ProgramHeader is
// defined in a separate file.
//
#include "elfz/public/elfW.h"
#include "elfz/public/selector.h"

namespace ELF
{
    /**
     * Interface to Elf headers
     */
    class ElfHdr : public Selector
    {
    public:
        ElfW(Half)  type() const;
        ElfW(Half)  machine() const;
        ElfW(Word)  version() const;
        ElfW(Addr)  entry() const;
        ElfW(Off)   phoff() const;
        ElfW(Off)   shoff() const;
        ElfW(Word)  flags() const;
        ElfW(Half)  ehsize() const;
        ElfW(Half)  phentsize() const;
        ElfW(Half)  phnum() const;
        ElfW(Half)  shentsize() const;
        ElfW(Half)  shnum() const;
        ElfW(Half)  shstrndx() const;

        Elf_Class klass() const { return Selector::klass(); }

        explicit ElfHdr(Elf*);

        bool is_null() const { return hdr32_ == 0; }

    private:
        union
        {
            Elf32_Ehdr* hdr32_;
            Elf64_Ehdr* hdr64_;
        };
    };


    /**
     * Adapt 32-bit section headers into 64-bit headers i
     */
    class SectionHdr : public Selector
    {
    public:
        friend class Section;
        friend class SymbolTable;

        const char* name() const;

        ElfW(Word)  type() const;
        ElfW(Xword) flags() const;
        ElfW(Addr)  addr() const;
        ElfW(Off)   offset() const;
        ElfW(Xword) size() const;
        ElfW(Word)  link() const;
        ElfW(Word)  info() const;
        ElfW(Xword) addralign() const;
        ElfW(Xword) entsize() const;

        SectionHdr(Elf*, Elf_Scn*);

        bool is_null() const { return hdr32_ == 0; }

    private:
        union
        {
            Elf32_Shdr* hdr32_;
            Elf64_Shdr* hdr64_;
        };
    };
}
#endif // HEADERS_H__48AF820C_8F74_453F_B098_F35A42BD9232
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
