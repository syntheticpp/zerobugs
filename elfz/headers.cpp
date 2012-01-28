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
#include <sstream>
#include "public/error.h"
#include "public/headers.h"

using namespace std;
using namespace ELF;

#define SELECT(f) ((klass() == ELFCLASS32) ? hdr32_->f : hdr64_->f)
#define RETURN(f) assert(!is_null()); return SELECT(f)



ElfHdr::ElfHdr(Elf* elf) : Selector(elf), hdr64_(0)
{
    if (elf)
    {
        switch (klass())
        {
        case ELFCLASS32:
            hdr32_ = elf32_getehdr(const_cast<Elf*>(elf));
            break;

        case ELFCLASS64:
            hdr64_ = elf64_getehdr(const_cast<Elf*>(elf));
            break;

        default:
            assert(false);
        }
    }

    if (is_null())
    {
        throw Error("elfxx_getehdr");
    }
}


ElfW(Half) ElfHdr::type() const
{
    RETURN(e_type);
}


ElfW(Half) ElfHdr::machine() const
{
    RETURN(e_machine);
}


ElfW(Word) ElfHdr::version() const
{
    RETURN(e_version);
}


ElfW(Addr) ElfHdr::entry() const
{
    RETURN(e_entry);
}


ElfW(Off) ElfHdr::phoff() const
{
    RETURN(e_phoff);
}


ElfW(Off) ElfHdr::shoff() const
{
    RETURN(e_shoff);
}


ElfW(Word) ElfHdr::flags() const
{
    RETURN(e_flags);
}


ElfW(Half) ElfHdr::ehsize() const
{
    RETURN(e_ehsize);
}


ElfW(Half)  ElfHdr::phentsize() const
{
    RETURN(e_phentsize);
}


ElfW(Half) ElfHdr::phnum() const
{
    RETURN(e_phnum);
}


ElfW(Half) ElfHdr::shentsize() const
{
    RETURN(e_shentsize);
}


ElfW(Half) ElfHdr::shnum() const
{
    RETURN(e_shnum);
}


ElfW(Half)  ElfHdr::shstrndx() const
{
    RETURN(e_shstrndx);
}


SectionHdr::SectionHdr(Elf* elf, Elf_Scn* scn)
    : Selector(elf)
    , hdr64_(0)
{
    if (scn)
    {
        switch (klass())
        {
        case ELFCLASS32:
            hdr32_ = elf32_getshdr(scn);
            break;

        case ELFCLASS64:
            hdr64_ = elf64_getshdr(scn);
            break;

        // Selector ctor should've taken care of this
        default: assert(false);
        }

        if (is_null())
        {
            throw Error("elfxx_getshdr");
        }
    }
}


const char* SectionHdr::name() const
{
    assert(!is_null());

    ElfHdr eh(elf());

    return elf_strptr(const_cast<Elf*>(elf()), eh.shstrndx(), SELECT(sh_name));
}


ElfW(Word)  SectionHdr::type() const
{
    RETURN(sh_type);
}


ElfW(Xword) SectionHdr::flags() const
{
    RETURN(sh_flags);
}


ElfW(Addr) SectionHdr::addr() const
{
    RETURN(sh_addr);
}


ElfW(Off) SectionHdr::offset() const
{
    RETURN(sh_offset);
}


ElfW(Xword) SectionHdr::size() const
{
    RETURN(sh_size);
}


ElfW(Word)  SectionHdr::link() const
{
    RETURN(sh_link);
}


ElfW(Word)  SectionHdr::info() const
{
    RETURN(sh_info);
}


ElfW(Xword) SectionHdr::addralign() const
{
    RETURN(sh_addralign);
}


ElfW(Xword) SectionHdr::entsize() const
{
    RETURN(sh_entsize);
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
