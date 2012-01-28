dnl http://autoconf-archive.cryp.to/ax_python.html
dnl
dnl sets PYTHON_BIN to the name of the python executable,
dnl PYTHON_INCLUDE_DIR to the directory holding the header files,
dnl and PYTHON_LIB to the name of the Python library. 
dnl
dnl LICENSE
dnl  Copyright © 2008 Michael Tindal
dnl 
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published
dnl by the Free Software Foundation; either version 2 of the License,
dnl or (at your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
dnl General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program. If not, see <http://www.gnu.org/licenses/>.
dnl 
dnl As a special exception, the respective Autoconf Macro's copyright
dnl owner gives unlimited permission to copy, distribute and modify the
dnl configure scripts that are the output of Autoconf when processing
dnl the Macro. You need not follow the terms of the GNU General Public
dnl License when using or distributing such scripts, even though
dnl portions of the text of the Macro appear in them. The GNU General
dnl Public License (GPL) does govern all other use of the material that
dnl constitutes the Autoconf Macro.
dnl 
dnl This special exception to the GPL applies to versions of the
dnl Autoconf Macro released by the Autoconf Macro Archive. When you
dnl make and distribute a modified version of the Autoconf Macro,
dnl you may extend this special exception to the GPL to apply to your
dnl modified version as well.
dnl 
AC_DEFUN([AX_PYTHON],
[AC_MSG_CHECKING(for python build information)
AC_MSG_RESULT([])
for python in python2.7 python2.6 python2.5 python2.4 python2.3 python2.2 python2.1 python; do
AC_CHECK_PROGS(PYTHON_BIN, [$python])
ax_python_bin=$PYTHON_BIN
if test x$ax_python_bin != x; then
   AC_CHECK_LIB($ax_python_bin, main, ax_python_lib=$ax_python_bin, ax_python_lib=no)
   AC_CHECK_HEADER([$ax_python_bin/Python.h],
   [[ax_python_header=`locate $ax_python_bin/Python.h | sed -e s,/Python.h,,`]],
   ax_python_header=no)
   if test "$ax_python_lib" != no; then
     if test "$ax_python_header" != no; then
       break;
     fi
   fi
fi
done
if test x$ax_python_bin = x; then
   ax_python_bin=no
fi
if test x$ax_python_header = x; then
   ax_python_header=no
fi
if test x$ax_python_lib = x; then
   ax_python_lib=no
fi

AC_MSG_RESULT([  results of the Python check:])
AC_MSG_RESULT([    Binary:      $ax_python_bin])
AC_MSG_RESULT([    Library:     $ax_python_lib])
AC_MSG_RESULT([    Include Dir: $ax_python_header])

if test x$ax_python_header != xno; then
  PYTHON_INCLUDE_DIR=$ax_python_header
  AC_SUBST(PYTHON_INCLUDE_DIR)
fi
if test x$ax_python_lib != xno; then
  PYTHON_LIB=$ax_python_lib
  AC_SUBST(PYTHON_LIB)
fi
])dnl

