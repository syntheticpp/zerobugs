#ifndef DEMANGLE_D_UTIL_H
#define DEMANGLE_D_UTIL_H 1

#include "config.h"

#undef DEMANGLE_D_REQUIRE_strndup
#undef DEMANGLE_D_REQUIRE_strtol_10
#undef DEMANGLE_D_REQUIRE_malloc
#undef DEMANGLE_D_REQUIRE_realloc
#undef DEMANGLE_D_REQUIRE_memmove
#undef DEMANGLE_D_REQUIRE_error

#ifdef DEMANGLE_D_IN_VALGRIND
/* gdb - http://www.gnu.org/software/gdb/ */
#error XXX
#define xstrndup        DD_(strndup)
#define DEMANGLE_D_REQUIRE_strndup 1
#define xstrtol_10      DD_(strtol_10)
#define DEMANGLE_D_REQUIRE_strtol_10 1
#define xmalloc         VG_(malloc)
#define xrealloc        VG_(realloc)
#define xmemmove        DD_(memmove)
#define DEMANGLE_D_REQUIRE_memmove 1
#define xfree           VG_(free)
#define xmemcpy         VG_(memcpy)

#elif defined(DEMANGLE_D_IN_GDB) /* not DEMANGLE_D_IN_VALGRIND */
/* gdb - http://www.gnu.org/software/gdb/ */
#error XXX
#include "../libiberty.h"
/* xmalloc */
/* xrealloc */

#include "../defs.h"
/* xfree */

#include <string.h>
#define xmemcpy		memcpy
#define xmemmove	memmove
#define xstrndup	strndup
#define xstrtol_10(n,p)	strtol((n), (p), 10)

#else  /* not DEMANGLE_D_IN_VALGRIND && not DEMANGLE_D_IN_GDB */
/* 'normal' libc */

#include <stdlib.h>
#include <string.h>

#ifdef __FreeBSD__
 #define DEMANGLE_D_REQUIRE_strndup 1
 #define xstrndup	strndup
#elif defined(__USE_GNU) || defined(_GNU_SOURCE)
 #define xstrndup	strndup
#else
#define xstrndup	DD_(strndup)
 #define DEMANGLE_D_REQUIRE_strndup 1
#endif

#define xstrtol_10(n,p) strtol((n), (p), 10)

#define xmalloc		DD_(malloc)
//#define DEMANGLE_D_REQUIRE_malloc 1
#define xrealloc	DD_(realloc)
//#define DEMANGLE_D_REQUIRE_realloc 1
#define xmemmove	memmove
#define xfree		free
#define xmemcpy		memcpy

#ifdef DEMANGLE_D_STANDALONE
//#define DEMANGLE_D_REQUIRE_error 1
#define xprintf		printf
#define xperror		perror
#define xfprintf	fprintf
#endif

#endif  /* not DEMANGLE_D_IN_VALGRIND && not DEMANGLE_D_IN_GDB */

#ifdef DEMANGLE_D_REQUIRE_strndup
char* DD_(strndup)(const char*, size_t);
#endif

#ifdef DEMANGLE_D_REQUIRE_strtol_10
long int DD_(strtol_10)(char*, char**);
#endif

#ifdef DEMANGLE_D_REQUIRE_malloc
void* DD_(malloc)(size_t);
#endif

#ifdef DEMANGLE_D_REQUIRE_realloc
void* DD_(realloc)(void*, size_t);
#endif

#ifdef DEMANGLE_D_REQUIRE_memmove
void * DD_(memmove)(void*, const void *, size_t );
#endif

#ifdef DEMANGLE_D_REQUIRE_error
void DD_(error)(const char*);
#endif

#endif /* DEMANGLE_D_UTIL_H */
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
