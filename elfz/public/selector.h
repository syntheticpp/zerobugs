#ifndef SELECTOR_H__F01EDCE8_4F08_4BCB_BE67_59F4871BE9DC
#define SELECTOR_H__F01EDCE8_4F08_4BCB_BE67_59F4871BE9DC
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

#include "elfz/public/elf.h"


/* #if ((__x86_64__) || (__WORDSIZE == 64)) && !__LIBELF64
# error incorrect configuration
#endif */

// todo: use configure script to determine whether Elf_Class defined
typedef int Elf_Class;


namespace ELF
{
    /**
     * Base class for selecting between
     * ELFCLASS32 (elf32_...) and ELFCLASS64 (elf64_...)
     * libelf functions and data structures.
     */
    class Selector
    {
    protected:
        explicit Selector(Elf*);
        Selector();

    public:
        Elf_Class klass() const { return klass_; }

        Elf* elf() const { return elf_; }

    private:
        Elf* elf_;
        Elf_Class klass_;
    };
}

#endif // SELECTOR_H__F01EDCE8_4F08_4BCB_BE67_59F4871BE9DC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
