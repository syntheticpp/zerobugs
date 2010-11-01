//
// $Id: selector.cpp 711 2010-10-16 07:09:23Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sstream>
#include <stdexcept>
#include "dharma/environ.h"
#include "public/error.h"
#include "public/selector.h"


ELF::Selector::Selector() : elf_(0), klass_(ELFCLASSNONE)
{
}


ELF::Selector::Selector(Elf* elf) : elf_(elf), klass_(ELFCLASSNONE)
{
    assert(elf);

    if (const char* ident = elf_getident(const_cast<Elf*>(elf), 0))
    {
        if (ident[EI_MAG0] != ELFMAG0
         || ident[EI_MAG1] != ELFMAG1
         || ident[EI_MAG2] != ELFMAG2
         || ident[EI_MAG3] != ELFMAG3)
        {
            throw std::runtime_error("not a valid ELF file");
        }
        klass_ = static_cast<Elf_Class>(ident[EI_CLASS]);
    }
    else
    {
    #if DEBUG
        if (env::get_bool("ZERO_ABORT_ON_ERROR", false))
        {
            abort();
        }
    #endif
        throw Error("elf_getident");
    }
    if (klass_ != ELFCLASS32 && klass_ != ELFCLASS64)
    {
        std::ostringstream errmsg;

        errmsg << "unhandled ELF class: " << (int)klass_;
        throw std::runtime_error(errmsg.str());
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
