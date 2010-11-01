#ifndef PROGRAM_HEADER_H__8E5E2D3B_0EFF_4FEC_8B01_4940ED674C7C
#define PROGRAM_HEADER_H__8E5E2D3B_0EFF_4FEC_8B01_4940ED674C7C
//
// $Id: program_header.h 714 2010-10-17 10:03:52Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "elfz/public/iterator.h"
#include "elfz/public/selector.h"


namespace ELF
{
    class ProgramHeader : public Selector
    {
    public:
        explicit ProgramHeader(Elf* elf);

        // ProgramHeader() : hdr32_(0), index_(0) { }

        ElfW(Word)  type() const;
        ElfW(Word)  flags() const;
        ElfW(Off)   offset() const;
        ElfW(Addr)  vaddr() const;
        ElfW(Addr)  paddr() const;
        ElfW(Xword) filesz() const;
        ElfW(Xword) memsz() const;
        ElfW(Xword) align() const;

        bool is_null() const { return hdr32_ == 0; }

        bool operator==(const ProgramHeader& other) const
        {
            return hdr64_ == other.hdr64_;
        }

        boost::shared_ptr<ProgramHeader> next() const;

    private:

        ProgramHeader(const ProgramHeader&, void*, size_t);

        boost::shared_ptr<ProgramHeader>
            next(const Binary::Header&) const;

        union
        {
            Elf32_Phdr* hdr32_;
            Elf64_Phdr* hdr64_;
        };

        size_t index_;
    };


    /* Specialize traits for traversing ELF program headers */
    template<> struct IterationTraits<ProgramHeader>
    {
        static boost::shared_ptr<ProgramHeader> first(File&);

        static boost::shared_ptr<ProgramHeader>
            next(File*, const ProgramHeader& phdr)
        {
            return phdr.next();
        }
    };
}

#endif // PROGRAM_HEADER_H__8E5E2D3B_0EFF_4FEC_8B01_4940ED674C7C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
