#                                               -*- Autoconf -*-
# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
# Process this file with autoconf to produce a configure script.
#
# TODO: check for python devel
#
AC_PREREQ(2.59)
m4_include(ax_boost_base.m4)
m4_include(ax_python.m4)
m4_include(ax_boost_python.m4)
m4_include(shell.m4)

AC_INIT(The-Zero-Debugger, 1.19, http://www.zerobugs.org)

AC_CONFIG_SRCDIR([../../stabz/parse_state.cpp])
AC_CONFIG_HEADER([../include/zdk/auto/config.h])
AC_PROG_SHELL
AC_LANG(C++)
AX_BOOST_BASE(1.32)
AX_PYTHON
if test -z "$PYTHON_INCLUDE_DIR"; then
  :
  #echo "Python development package required"
  #exit 2
else
  AC_DEFINE([HAVE_PYTHON], [1], [Define if Python is installed])
fi
AX_BOOST_PYTHON

# Checks for programs.
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LEX
if test "$LEX" != flex; then
  echo "flex required"
  exit 2
fi
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB


AC_DEFUN([AC_YYFLEXLEXER_YYWRAP],
    [AC_CACHE_CHECK([for yyFlexLexer::yywrap],
               [ac_cv_yyFlexLexer_yywrap],
               [ac_cv_yyFlexLexer_yywrap=false
cat > tmp.cpp << ---end---
#include <FlexLexer.h>

int main()
{
    int (yyFlexLexer::*method)() = &yyFlexLexer::yywrap;
    return 0;
}
---end---
    if g++ tmp.cpp 2>/dev/null; then 
        ac_cv_yyFlexLexer_yywrap=true
        rm a.out
    fi
    rm tmp.cpp])

    if test "$ac_cv_yyFlexLexer_yywrap" = true; then
        AC_DEFINE([HAVE_YYFLEXLEXER_YYWRAP], [1],
                [Define if yyFlexLexer::yywrap is declared])
    fi
])

AC_YYFLEXLEXER_YYWRAP


# Checks for libraries.
AC_CHECK_LIB([termcap],[tputs], 
[
  if test -f /usr/lib/libtermcap.a; then
    AC_SUBST(LIBTERMCAP,/usr/lib/libtermcap.a) 
  else
    AC_SUBST(LIBTERMCAP,-ltermcap)
  fi
], [])

AC_CHECK_LIB([dl], [dlsym], AC_SUBST(LIBDL,-ldl))
            
AC_CHECK_LIB([elf], [elf_version], [], [exit 1], [-lpthread])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([minielf], [elf_errno], AC_SUBST(LIBELF, -lminielf), AC_SUBST(LIBELF, -lelf))


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT

AC_CHECK_HEADERS([fcntl.h libgen.h libintl.h limits.h mach/mach.h malloc.h stddef.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/sysinfo.h sys/time.h termcap.h termios.h unistd.h uuid/uuid.h uuid.h])


AC_CHECK_HEADER([proc_service.h],
                [AC_DEFINE([HAVE_PROC_SERVICE_H], [1],
                    [Define to 1 if you have <proc_service.h>.])],
                [])
AC_CHECK_HEADER([thread_db.h],
                [AC_DEFINE([HAVE_THREAD_DB_H], [1],
                    [Define to 1 if you have <thread_db.h>.])],
                [])
AC_CHECK_HEADER([unistd.h],
                [AC_DEFINE([HAVE_UNISTD_H], [1],
                    [Define to 1 if you have <unistd.h>.])],
                [])
AC_CHECK_HEADER([asm/ptrace.h],
                [AC_DEFINE([HAVE_ASM_PTRACE_H], [1],
                    [Define to 1 if you have <asm/ptrace.h>.])],
                [])
AC_CHECK_HEADER([asm/prctl.h],
                [AC_DEFINE([HAVE_ASM_PRCTL_H], [1],
                    [Define to 1 if you have <asm/prctl.h>.])],
                [])
AC_CHECK_HEADER([asm/ldt.h],
                [AC_DEFINE([HAVE_ASM_LDT_H], [1],
                    [Define to 1 if you have <asm/ldt.h>.])],
                [])
AC_CHECK_HEADER([asm/unistd.h],
                [AC_DEFINE([HAVE_ASM_UNISTD_H], [1],
                    [Define to 1 if you have <asm/unistd.h>.])],
                [])
AC_CHECK_HEADER([atomic_ops.h],
                [AC_DEFINE([HAVE_ATOMIC_OPS_H], [1],
                    [Define to 1 if you have <atomic_ops.h>.])],
                    [echo "Please make sure libatomic-ops-dev is installed"; exit 1]
                )
AC_CHECK_HEADER([libelf.h],
                [AC_DEFINE([HAVE_LIBELF_H], [1],
                    [Define to 1 if you have <libelf.h>.])],
                [])
AC_CHECK_HEADER([libelf/libelf.h],
                [AC_DEFINE([HAVE_LIBELF_LIBELF_H], [1],
                    [Define to 1 if you have <libelf/libelf.h>.])],
                [])
AC_CHECK_HEADER([sys/param.h],
                [AC_DEFINE([HAVE_SYS_PARAM_H], [1],
                    [Define to 1 if you have <sys/param.h>.])],
                [])
AC_CHECK_HEADER([sys/procfs.h],
                [AC_DEFINE([HAVE_SYS_PROCFS_H], [1],
                    [Define to 1 if you have <sys/procfs.h>.])],
                [])
AC_CHECK_HEADER([sys/ptrace.h],
                [AC_DEFINE([HAVE_SYS_PTRACE_H], [1],
                    [Define to 1 if you have <sys/ptrace.h>.])],
                [])
AC_CHECK_HEADER([sys/syscall.h],
                [AC_DEFINE([HAVE_SYS_SYSCALL_H], [1],
                    [Define to 1 if you have <sys/syscall.h>.])],
                [])
AC_CHECK_HEADER([sys/user.h],
                [AC_DEFINE([HAVE_SYS_USER_H], [1],
                    [Define to 1 if you have <sys/user.h>.])],
                [])
AC_CHECK_HEADER([sys/wait.h],
                [AC_DEFINE([HAVE_SYS_WAIT_H], [1],
                    [Define to 1 if you have <sys/wait.h>.])],
                [])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])

AC_CHECK_TYPES([struct user_fpregs_struct, struct user_fpxregs_struct],[],[],
[
    #include <sys/types.h>
    #include <sys/user.h>
])
AC_CHECK_TYPES([modify_ldt_t, struct user_desc],[],[],[#include <asm/ldt.h>])
AC_CHECK_TYPES([thread_t],[],[],[#include <thread_db.h>])

AC_CHECK_TYPES([__uint32_t],[],[],[#include <sys/types.h>])
AC_CHECK_TYPES([__uint64_t],[],[],[#include <sys/types.h>])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
###AC_REPLACE_FNMATCH

AC_FUNC_FORK
AC_FUNC_GETPGRP
AC_PROG_GCC_TRADITIONAL
###AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
###AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_CHECK_FUNCS([atexit dup2 ftruncate getcwd getpagesize memset mkdir munmap pow realpath rmdir select setenv strchr strdup strerror strncasecmp strrchr strstr strtol strtoul strtoull uname])
#AC_CHECK_FUNC([uuid_create],[],[],["#include <uuid.h>]")
#AC_CHECK_FUNC([uuid_generate],[],["#include <uuid/uuid.h>]")


dnl check for unordered_map
AC_MSG_CHECKING(for unordered_map)
AC_TRY_COMPILE([#include <unordered_map>
#include <string>], [std::unordered_map<std::string, std::string> m; m["foo"]="bar";], have_unordered_map=yes, have_unordered_map=no)
if test "$have_unordered_map" = yes; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_UNORDERED_MAP,,[support for unordered_map detected])
else
    AC_MSG_RESULT(no)
fi

AC_CONFIG_FILES([Common.mak Python.mak ../../Makefile ../../engine/Makefile ../../plugin/python/Makefile])

AC_OUTPUT

cd ../../plugin/gui

GNOME_LIBDIR=`gnome-config --libdir 2>/dev/null`
if test -n "$GNOME_LIBDIR"; then
	LDFLAGS=-L$GNOME_LIBDIR ./configure
else
	if ! ./configure; then
        exit 1
    fi
fi

