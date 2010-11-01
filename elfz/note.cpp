//
// $Id: note.cpp 711 2010-10-16 07:09:23Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "public/file.h"
#include "public/note.h"
#include "private/align.h"

using namespace std;
using namespace ELF;

#define SELECT(f) ((klass() == ELFCLASS32) ? note32_->f : note64_->f)
#define RETURN(f) assert(!is_null()); return SELECT(f)


Note::Note(const File& elf, const unsigned char* data)
    : Selector(const_cast<Elf*>(elf.elf()))
    , note64_(0)
    , totalRoundedSize_(0)
{
    switch (klass())
    {
    case ELFCLASS32:
        note32_ = (Elf32_Nhdr*)data;
        totalRoundedSize_ = sizeof(Elf32_Nhdr);
        break;

    case ELFCLASS64:
        note64_ = (Elf64_Nhdr*)data;
        totalRoundedSize_ = sizeof(Elf64_Nhdr);
        break;

    default:
        assert(false);
    }

    name_.assign((const char*)data + totalRoundedSize_, namesz());

    totalRoundedSize_ += align(namesz(), 2);
    const unsigned char* begin = data + totalRoundedSize_;

    data_.assign(begin, begin + descsz());

    totalRoundedSize_ += align(descsz(), 2);
}


ElfW(Word) Note::namesz() const
{
    RETURN(n_namesz);
}


ElfW(Word) Note::descsz() const
{
    RETURN(n_descsz);
}


ElfW(Word) Note::type() const
{
    RETURN(n_type);
}


const char* Note::name() const
{
    return name_.c_str();
}



// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
