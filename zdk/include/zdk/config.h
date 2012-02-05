#ifndef CONFIG_H__900787A1_4E20_11DA_A1C0_00C04F09BBCC
#define CONFIG_H__900787A1_4E20_11DA_A1C0_00C04F09BBCC
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
// enable debugging 32-bit apps on 64-bit hosts
#define _XDEBUG_32_ON_64    1

/////////////////////////////////////////////////////////////////

// on my x86-64 Linux, config.h messes up cstdlib if included before
#include <cstdlib>
#if !defined(WIN32)
#include "zdk/auto/config.h" // configure-generated
#endif

#include <stdint.h>

#if defined(__GNUC__)
 #define BUILTIN_MEMCPY __builtin_memcpy

#else
 #define __attribute__(a) // as nothing
 #define BUILTIN_MEMCPY memcpy
#endif

#ifdef HAVE_SYS_TYPES_H
 #include <sys/types.h>
#endif
#if defined(linux) || defined(__linux__)
 #include "zdk/config/linux.h"
#elif defined(__FreeBSD__)
 #include "zdk/config/bsd.h"
#elif defined(__APPLE__)
 #include "zdk/config/darwin.h"
#endif

#if !defined(HAVE_ELF32_XWORD)
 typedef uint64_t Elf32_Xword;
#endif
#if !defined(HAVE_ELF64_XWORD)
 typedef uint64_t Elf64_Xword;
#endif

#if !defined(HAVE_STRUCT_USER_FPXREGS_STRUCT)
// just fallback to using fpregs
 #define user_fpxregs_struct user_fpregs_struct
#endif

#if defined(__i386__) || defined(__x86_64__)
 #define HAVE_DEBUG_REGS_386 1
#endif

#if !defined(HAVE_MODIFY_LDT_T) && defined(HAVE_STRUCT_USER_DESC)
 typedef struct user_desc modify_ldt_t;
#endif


#if !defined(HAVE_SETENV)

 #include <errno.h>
 #include <stdlib.h>
 #include <string>

 inline int setenv(const char* envName, const char* envVal, int /* overwrite */)
 {
	 try
	 {
		std::string envStr = envName;
		envStr += "=";
		envStr += envVal;
		
		return _putenv(envStr.c_str());
	 }
	 catch (const std::bad_alloc&)
	 {
		 return ENOMEM;
	 }
 }

#endif // !defined(HAVE_SETENV)
#endif // CONFIG_H__900787A1_4E20_11DA_A1C0_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
