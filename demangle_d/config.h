#ifndef DEMANGLE_D_CONFIG_H
#define DEMANGLE_D_CONFIG_H 1

#include "demangle.h"

/* for propper strtold support */
#undef _ISOC99_SOURCE 
#define _ISOC99_SOURCE 1

#undef DEMANGLE_D_REQUIRE_ISDIGIT
#undef DEMANGLE_D_REQUIRE_ISXDIGIT
#undef DEMANGLE_D_REQUIRE_ASCI2HEX

#ifdef DEMANGLE_D_IN_VALGRIND

/* valgrind - http://www.valgrind.org */

#include <stddef.h> /* size_t */

#define xstrlen		VG_(strlen)
#define xstrncmp	VG_(strncmp)
#define xsnprintf	VG_(snprintf)
#define xisdigit	ISDIGIT
#define DEMANGLE_D_REQUIRE_ISDIGIT 1
#define xisxdigit	ISXDIGIT
#define DEMANGLE_D_REQUIRE_ISXDIGIT 1
#define xasci2hex	ASCI2HEX
#define DEMANGLE_D_REQUIRE_ASCI2HEX 1


#elif defined(DEMANGLE_D_IN_GDB) /* not DEMANGLE_D_IN_VALGRIND */

/* gdb - http://www.gnu.org/software/gdb/ */

#include <stddef.h> /* size_t */

#include <string.h>
#define xstrlen		strlen
#define xstrncmp	strncmp


#include "../defs.h"
/* xsnprintf */

#include "../../include/safe-ctype.h"
#define xisdigit	ISDIGIT
#define xisxdigit	ISXDIGIT

#define xasci2hex	ASCI2HEX
#define DEMANGLE_D_REQUIRE_ASCI2HEX 1


#else /* not DEMANGLE_D_IN_GDB */

/* "normal" libc */

#include <stddef.h> /* size_t */

#include <string.h>
#define xstrlen		strlen
#define xstrncmp	strncmp

#include <stdlib.h>
#define xabort		abort

#include <stdio.h>
#define xsnprintf	snprintf

#include <ctype.h>
#define xisdigit	isdigit
#define xisxdigit	isxdigit

#define xasci2hex	ASCI2HEX
#define DEMANGLE_D_REQUIRE_ASCI2HEX 1


#endif /* not DEMANGLE_D_IN_VALGRIND && not DEMANGLE_D_IN_GDB */

#ifndef _LIBINTL_H
#define _(str)		str
#endif

/* helper macros */

#ifdef DEMANGLE_D_REQUIRE_ISDIGIT
#define ISDIGIT(c) (('0' <= (c)) && ((c) <= '9'))
#endif

#ifdef DEMANGLE_D_REQUIRE_ISXDIGIT
#define ISXDIGIT(c) ( \
		(('0' <= (c)) && ((c) <= '9')) \
		|| (('a' <= (c)) && ((c) <= 'f')) \
		|| (('A' <= (c)) && ((c) <= 'F')) \
		)
#endif

#ifdef DEMANGLE_D_REQUIRE_ASCI2HEX
#define ASCI2HEX(c) \
	( \
	 	('a' <= (c) && (c) <= 'f') \
		? \
		((c) - 'a' + 10) \
		: \
		( \
		 	('A' <= (c) && (c) <= 'F') \
			? \
			((c) - 'A' + 10) \
			: \
			( \
			 	('0' <= (c) && (c) <= '9') \
				? \
				((c) - '0') \
				: \
				0 \
			) \
		) \
	)
#endif

#endif /* DEMANGLE_D_CONFIG_H */
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
