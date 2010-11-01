#ifndef ELF_W_H__1056A7E8_8F57_4F97_82CE_60466A0C393A
#define ELF_W_H__1056A7E8_8F57_4F97_82CE_60466A0C393A
//
// $Id: elfW.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "elfz/public/elf.h"

#ifndef ElfW
 #if (__ELF_WORD_SIZE == 64) || (__WORDSIZE == 64)
   #define ElfW(x) Elf64_##x
 #elif defined(__i386__)
   #define ElfW(x) Elf32_##x
 #elif defined (__PPC__)
   // 32bit Power PC
   #define ElfW(x) Elf32_##x
 #endif
#endif // !ElfW
#endif // ELF_W_H__1056A7E8_8F57_4F97_82CE_60466A0C393A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
