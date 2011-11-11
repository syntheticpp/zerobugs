#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
#AC_PREREQ(2.59)
AC_INIT(The-Zero-Debugger, 1.19, http://www.zerobugs.org)

AC_LANG(C++)

AC_CONFIG_SRCDIR([views_factory.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([z], [deflate])

PKG_CHECK_MODULES(X11, x11,[],[echo "Makefile will use best guess"])
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

if test -z "$X11_LIBS"; then
    AC_PATH_XTRA
    LDFLAGS="$LDFLAGS $X_LIBS"
    LIBS="$X_PRE_LIBS $X_EXTRA_LIBS $LIBS"
else
    LIBS="$X11_LIBS $LIBS"
fi

dnl AC_CHECK_LIB([Xpm], [XpmAttributesSize],[],[exit 1], [-lX11 -lXext])

PKG_CHECK_EXISTS(gtkmm-2.4, 
    [AC_DEFINE(GTKMM_2, [1], [Define to 1 if using GTKMM_2])
     AC_SUBST(GTKMM_2, [1])])

PKG_CHECK_MODULES(GTKMM, gtkmm-2.4 gtksourceview-1.0,
    [
        AC_SUBST(GTKSVLIB,gtksourceview-1.0)
    ],
    [
        PKG_CHECK_MODULES(GTKMM, gtkmm-2.4 gtksourceview-2.0,
        [
            AC_SUBST(GTKSVLIB,gtksourceview-2.0)
        ],
        [echo "either gtkmm-2.4 or gtksourceview is missing" && exit 1])
    ])

dnl AC_CHECK_LIB([gtksourceview-1.0],[gtk_source_view_new],[],[exit -1])
dnl AC_CHECK_FUNC([gtk_source_view_set_highlight_current_line],[],[exit -1])
AC_CHECK_LIB([$GTKSVLIB],[gtk_source_view_set_highlight_current_line],
[
    AC_DEFINE([HAVE_GTK_SOURCE_VIEW_SET_HIGHLIGHT_CURRENT_LINE], [1], 
              [Define to 1 if have gtk_source_view_set_highlight_current_line])
],[],[]
)


PKG_CHECK_MODULES(CAIROMM, cairomm-1.0, [], [echo cairomm not found])
AC_SUBST(CAIROMM_CFLAGS)
AC_SUBST(CAIROMM_LIBS)

dnl on some systems such as Debian, 
dnl the package is called libgtksourceviewmm
PKG_CHECK_MODULES(GTKSOURCEVIEWMM, gtksourceviewmm-1.0, 
    [
     AC_DEFINE(GTKSVMM_API_VERSION, [1],[gtksourceviewmm version])
    ],
    [PKG_CHECK_MODULES(GTKSOURCEVIEWMM, libgtksourceviewmm-1.0, 
       [],
       [PKG_CHECK_MODULES(GTKSOURCEVIEWMM, gtksourceviewmm-2.0, 
            [
                AC_DEFINE(GTKSVMM_API_VERSION, [2],
                          [gtksourceviewmm version]
                         )
                ac_cv_source_languages_manager=true
            ])
       ])
    ])
AC_SUBST(GTKSOURCEVIEWMM_CFLAGS)

GTKMM_CFLAGS="$GTKMM_CFLAGS $GTKSOURCEVIEWMM_CFLAGS"
GTKMM_LIBS="$GTKMM_LIBS $GTKSOURCEVIEWMM_LIBS"

AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)


#Debian Sarge comes with 3.1
PKG_CHECK_MODULES(LIBGTKHTML, libgtkhtml-3.1, 
[
    AC_DEFINE([HAVE_LIBGTKHTML], [1], [Define to 1 if have libgtkhtml])
],
[
    #Ubuntu 7.04
    PKG_CHECK_MODULES(LIBGTKHTML, libgtkhtml-3.14, 
    [
        AC_DEFINE([HAVE_LIBGTKHTML], [1], [Define to 1 if have libgtkhtml])
        LIBGTKHTML_LIBS=-lgtkhtml-3.14
    ],
    [
        PKG_CHECK_MODULES(LIBGTKHTML, libgtkhtml-3.6, 
        [
            AC_DEFINE([HAVE_LIBGTKHTML], [1], [Define to 1 if have libgtkhtml])
        ],
        [
            PKG_CHECK_MODULES(LIBGTKHTML, libgtkhtml-3.8, 
            [
                AC_DEFINE([HAVE_LIBGTKHTML], [1], [Define to 1 if have libgtkhtml])
            ],
            [
                #for systems that have the dev files but miss the pkginfo:
                AC_CHECK_LIB([gtkhtml-3.8], [gtk_html_new],
                [
                    AC_SUBST(LIBGTKHTML_LIBS,-lgtkhtml-3.8) 
                    AC_SUBST(LIBGTKHTML_CFLAGS,-I/usr/include/libgtkhtml-3.8)
                    AC_DEFINE([HAVE_LIBGTKHTML], [1], [Define to 1 if have gtkhtml])
                ],[exit 1],[])
            ])
        ])
    ])
])

AC_SUBST(LIBGTKHTML_LIBS)
AC_SUBST(LIBGTKHTML_CFLAGS)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h string.h sys/time.h unistd.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memset select setenv strchr strerror strncasecmp strrchr strtol strtoul])

#check for deprecated cairo font
AC_DEFUN([AC_CAIRO_DEPRECATED],
    [AC_CACHE_CHECK([for CAIRO_FONT_TYPE_ATSUI],
               [ac_cv_cairo_font_type_atsui],
               [ac_cv_cairo_font_type_atsui=false
cat > tmp.cpp << ---end---
#include <cairo/cairo.h>

int main()
{
    int i = CAIRO_FONT_TYPE_ATSUI;
    return i;
}
---end---
if g++ -I/usr/include/cairo tmp.cpp 2>/dev/null; then
    ac_cv_cairo_font_type_atsui=true
    rm a.out
fi
rm tmp.cpp])
if test "$ac_cv_cairo_font_type_atsui" = true; then
        AC_DEFINE([HAVE_CAIRO_FONT_TYPE_ATSUI], [1],
            [Define if CAIRO_FONT_TYPE_ATSUI available])
else
        AC_SUBST(FONT_ATSUI,
CXXFLAGS+=-DCAIRO_FONT_TYPE_ATSUI_replaced_by_CAIRO_FONT_TYPE_QUARTZ=CAIRO_FONT_TYPE_QUARTZ)
    GTKMM_CFLAGS="$GTKMM_CFLAGS \
-DCAIRO_FONT_TYPE_ATSUI_replaced_by_CAIRO_FONT_TYPE_QUARTZ=CAIRO_FONT_TYPE_QUARTZ"
fi])

AC_CAIRO_DEPRECATED

#check for cairo in Gdk
AC_CACHE_CHECK([for GDK_CAIRO],ac_cv_gdk_cairo,[
  CXXFLAGS="$GTKMM_CFLAGS"
  LDFLAGS="$GTKMM_LIBS"
  AC_TRY_COMPILE([#include <gtkmm/main.h>
  #include <gtkmm/widget.h>],
 [return &Gdk::Window::create_cairo_context != 0],
 ac_cv_gdk_cairo=yes,ac_cv_gdk_cairo=no,)])
 if test "$ac_cv_gdk_cairo" = "yes"; then
    AC_DEFINE([HAVE_GDK_CAIRO_SUPPORT], [1], [Define if Gdk uses Cairo graphics])
 fi 

#check for new (2.12) tooltip API
AC_CACHE_CHECK([for new Gtkmm tooltips API],ac_cv_new_tooltip_api,[
  CXXFLAGS="$GTKMM_CFLAGS"
  LDFLAGS="$GTKMM_LIBS"
  AC_TRY_COMPILE([#include <gtkmm/widget.h>],
 [return &Gtk::Widget::set_has_tooltip != 0],
 ac_cv_new_tooltip_api=yes,ac_cv_new_tooltip_api=no,)])
 if test "$ac_cv_new_tooltip_api" = "yes"; then
    AC_DEFINE([HAVE_NEW_TOOLTIP_API], [1], [Define if Gtk has new tooltip API])
 fi 

#check for Notebook set_group
AC_CACHE_CHECK([for Gtk::Notebook::set_group],ac_cv_notebook_set_group,[
  CXXFLAGS="$GTKMM_CFLAGS"
  LDFLAGS="$GTKMM_LIBS"
  AC_TRY_COMPILE([#include <gtkmm/notebook.h>],
 [return &Gtk::Notebook::set_group != 0],
 ac_cv_notebook_set_group=yes,ac_cv_notebook_set_group=no,)])
 if test "$ac_cv_notebook_set_group" = "yes"; then
    AC_DEFINE([HAVE_NOTEBOOK_SET_GROUP], [1], [Define if Gtk::Notebook has set_group])
 fi 
#check for SourceLanguagesManager::create() in gtksourceviewmm-1.0
AC_DEFUN([AC_SOURCE_LANGUAGES_MANAGER_CREATE],
    [AC_CACHE_CHECK([for SourceLanguagesManager::create],
               [ac_cv_source_languages_manager],
               [ac_cv_source_languages_manager=false
    grep create /usr/include/gtksourceviewmm-1.0/gtksourceviewmm/sourcelanguagesmanager.h \
        2>/dev/null 1>&2 || \
    grep create /usr/local/include/gtksourceviewmm-1.0/gtksourceviewmm/sourcelanguagesmanager.h \
        2>/dev/null 1>&2 && ac_cv_source_languages_manager=true \
    && ac_cv_source_languages_manager=true])
    if test "$ac_cv_source_languages_manager" = true; then
        AC_DEFINE([HAVE_SOURCE_LANGUAGES_MANAGER_CREATE], [1],
                [Define if SourceLanguagesManager::create is declared])
    fi
    ])

AC_SOURCE_LANGUAGES_MANAGER_CREATE

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
