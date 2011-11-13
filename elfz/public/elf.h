#ifndef ELF_H__FE23F2E0_5058_11DA_88B7_000C29CB02FA
#define ELF_H__FE23F2E0_5058_11DA_88B7_000C29CB02FA
//
// $Id: elf.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#if HAVE_ELF_H
 #include <elf.h>
#endif

#if HAVE_LIBELF_LIBELF_H
 #include <libelf/libelf.h>
#elif HAVE_LIBELF_H
 #include <libelf.h>
#endif

#ifndef HAVE_ELF64_XWORD
 typedef uint64_t Elf64_Xword;
#endif

#ifndef HAVE_ELF64_SXWORD
 typedef int64_t Elf64_Sxword;
#endif

#if !HAVE_ELF_NHDR
/*
 * Note entry header
 */
typedef struct Elf32_Nhdr {
    Elf32_Word		n_namesz;	/* name size */
    Elf32_Word		n_descsz;	/* descriptor size */
    Elf32_Word		n_type;		/* descriptor type */
};

typedef struct Elf64_Nhdr {
    Elf64_Word		n_namesz;	/* name size */
    Elf64_Word		n_descsz;	/* descriptor size */
    Elf64_Word		n_type;		/* descriptor type */
};

#endif // HAVE_ELF_NHDR
#endif // ELF_H__FE23F2E0_5058_11DA_88B7_000C29CB02FA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
