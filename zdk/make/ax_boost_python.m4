dnl This macro checks to see if the Boost.Python library is installed.
dnl It also attempts to guess the currect library name using several attempts.
dnl It tries to build the library name using a user supplied name or suffix 
dnl and then just the raw library.
dnl
dnl If the library is found, HAVE_BOOST_PYTHON is defined and BOOST_PYTHON_LIB 
dnl is set to the name of the library.
dnl
dnl This macro calls AC_SUBST(BOOST_PYTHON_LIB).
dnl
dnl In order to ensure that the Python headers are specified on the include path,
dnl this macro requires AX_PYTHON to be called.
dnl
dnl
dnl LICENSE
dnl Copyright © 2008 Michael Tindal
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
AC_DEFUN([AX_BOOST_PYTHON],
[AC_REQUIRE([AX_PYTHON])dnl
AC_CACHE_CHECK(whether the Boost::Python library is available,
ac_cv_boost_python,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 CPPFLAGS_SAVE=$CPPFLAGS
 if test x$PYTHON_INCLUDE_DIR != x; then
   CPPFLAGS=-I$PYTHON_INCLUDE_DIR $CPPFLAGS
 fi
 AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[
 #include <boost/python/module.hpp>
 using namespace boost::python;
 BOOST_PYTHON_MODULE(test) { throw "Boost::Python test."; }]],
                           [[return 0;]]),
                           ac_cv_boost_python=yes, ac_cv_boost_python=no)
 AC_LANG_RESTORE
 CPPFLAGS=$CPPFLAGS_SAVE
])
if test "$ac_cv_boost_python" = "yes"; then
  AC_DEFINE(HAVE_BOOST_PYTHON,,[define if the Boost::Python library is available])
  ac_save_LDFLAGS="$LDFLAGS"
  LDFLAGS=-l"$ax_python_lib"
  ax_python_lib=boost_python
  AC_ARG_WITH([boost-python],AS_HELP_STRING([--with-boost-python],[specify the boost python library or suffix to use]),
  [if test "x$with_boost_python" != "xno"; then
     ax_python_lib=$with_boost_python
     ax_boost_python_lib=boost_python-$with_boost_python
   fi])
  for ax_lib in $ax_python_lib $ax_boost_python_lib boost_python; do
    AC_CHECK_LIB($ax_lib, exit, [BOOST_PYTHON_LIB=$ax_lib break])
  done
  LDFLAGS="$ac_save_LDFLAGS"
  AC_SUBST(BOOST_PYTHON_LIB)
fi
])dnl

