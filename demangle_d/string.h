#ifndef DEMANGLE_D_STRING_H
#define DEMANGLE_D_STRING_H 1

#include "config.h"

#undef DEMANGLE_D_USE_LIBIBERY

/*#ifdef DEMANGLE_D_IN_GDB

#include "../../include/dyn-string.h"
#define DEMANGLE_D_USE_LIBIBERY 1

#elif defined(DEMANGLE_D_IN_VALGRIND)

#include "../dyn-string.h"
#define DEMANGLE_D_USE_LIBIBERY 1

#endif*/

#ifdef DEMANGLE_D_USE_LIBIBERY

#define string_t	dyn_string_t	
#define new_string()	dyn_string(128)

#define append_n	DD_(append_n)
#define append_c	dyn_string_append_char
#define append		dyn_string_append_cstr

#define prepend_n	DD_(prepend_n)
#define prepend		dyn_string_prepend_cstr

#define nestpend_n	DD_(nestpend_n)
#define nestpend	DD_(nestpend)

#else /* not DEMANGLE_D_USE_LIBIBERY */

#define string_t	DD_(string_t)
#define string_t_r	DD_(string_t_r)
#define new_string	DD_(new_string)
#define append_n	DD_(append_n)
#define append_c	DD_(append_c)
#define append		DD_(append)
#define prepend_n	DD_(prepend_n)
#define prepend		DD_(prepend)
#define nestpend_n	DD_(nestpend_n)
#define nestpend	DD_(nestpend)

typedef struct{
	size_t	used;
	char*	str;
	size_t	len;
} string_t_r;

typedef string_t_r* string_t;

string_t new_string(void);

void append_c(string_t, int);
void append(string_t, const char *);
void prepend(string_t, const char *);

#endif /* not DEMANGLE_D_USE_LIBIBERY */

void append_n(string_t, const char *, size_t);
void prepend_n(string_t, const char *, size_t);

void nestpend_n(string_t, const char *, size_t, int);
void nestpend(string_t, const char*, int);

#endif /* DEMANGLE_D_STRING_H */
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
