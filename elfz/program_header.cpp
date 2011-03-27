//
// $Id: program_header.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdexcept>
#include "public/binary.h"
#include "public/error.h"
#include "public/headers.h"
#include "public/program_header.h"


#ifndef __GNUC__
 #define __PRETTY_FUNCTION__ __func__
#endif

using namespace std;
using namespace ELF;

#define SELECT(f) ((klass() == ELFCLASS32) ? hdr32_->f : hdr64_->f)
#define RETURN(f) assert(!is_null()); return SELECT(f)


ProgramHeader::ProgramHeader(Elf* elf)
    : Selector(elf)
    , hdr64_(0)
    , index_(0)
{
    switch (klass())
    {
    case ELFCLASS32:
        hdr32_ = elf32_getphdr(this->elf());
        break;

    case ELFCLASS64:
        hdr64_ = elf64_getphdr(this->elf());
        break;

    default:
        assert(false);
    }

    if (is_null())
    {
        throw Error("elfxx_getpshdr");
    }
}


ProgramHeader::ProgramHeader
(
    const ProgramHeader& prev,
    void* hdr,
    size_t index
)
    : Selector(prev)
    , hdr64_(reinterpret_cast<Elf64_Phdr*>(hdr))
    , index_(index)
{
}


boost::shared_ptr<ProgramHeader> ProgramHeader::next() const
{
    Binary::Header header(elf());
    return next(header);
}


boost::shared_ptr<ProgramHeader>
ProgramHeader::next(const Binary::Header& header) const
{
    if (is_null())
    {
        throw out_of_range(__PRETTY_FUNCTION__);
    }
    boost::shared_ptr<ProgramHeader> ph;

    if (index_ + 1 < header.phnum())
    {
        void* nxt = 0;

        switch (klass())
        {
        case ELFCLASS32:
            nxt = (hdr32_ + 1);
            break;

        case ELFCLASS64:
            nxt = (hdr64_ + 1);
            break;

        default:
            assert(false);
        }

        if (nxt)
        {
            boost::shared_ptr<ProgramHeader> p(
                new ProgramHeader(*this, nxt, index_ + 1));
            ph = p;
        }
    }
    return ph;
}


ElfW(Word) ProgramHeader::type() const
{
    RETURN(p_type);
}


ElfW(Word) ProgramHeader::flags() const
{
    RETURN(p_flags);
}


ElfW(Off) ProgramHeader::offset() const
{
    RETURN(p_offset);
}


ElfW(Addr) ProgramHeader::vaddr() const
{
    RETURN(p_vaddr);
}


ElfW(Addr) ProgramHeader::paddr() const
{
    RETURN(p_paddr);
}


ElfW(Xword) ProgramHeader::filesz() const
{
    RETURN(p_filesz);
}


ElfW(Xword) ProgramHeader::memsz() const
{
    RETURN(p_memsz);
}


ElfW(Xword) ProgramHeader::align() const
{
    RETURN(p_align);
}


boost::shared_ptr<ProgramHeader>
IterationTraits<ProgramHeader>::first(File& file)
{
    assert(file.elf());
    return boost::shared_ptr<ProgramHeader>(new ProgramHeader(file.elf()));
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
