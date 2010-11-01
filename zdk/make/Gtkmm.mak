# vim: tabstop=4:softtabstop=4:noexpandtab:shiftwidth=4 
# $Id: Gtkmm.mak 588 2008-08-06 04:50:35Z root $
# 
LIBDIR=lib
ifeq ($(ARCH),x86_64)
 LIBDIR=lib64
endif

ifeq ($(GTKMM_2),)
# configure does not handle ancient gtk stuff...
#
# gtk-- 1.2
#
# I assume that Gtkmm-1.2 is installed in /usr/local, because
# it has to be built locally, with the same C++ compiler that
# is used to build the rest of the project.

include $(TOP)/zdk/make/glibpath.mak

GTK_INCLUDE=\
	-I. \
	-I${GLIB_PREFIX}/${LIBDIR}/glib/include \
	-I${GLIB_PREFIX}/include/glib-1.2 \
	-I${GTK_PREFIX}/include/gtk-1.2 \
	-I${GTKMM_PREFIX}/${LIBDIR}/gtkmm/include \
	-I${GTKMM_PREFIX}/include \
	-I${SIGC_PREFIX}/include/sigc++ \
	-I${SIGC_PREFIX}/${LIBDIR}/sigc++/include

LDLIBS+=\
	-lgtkmm   \
	-lgdkmm   \
	-lsigc	  \
	-lgtk	  \
	-lgdk	  \
	-lglib	  \
	-lgmodule \
	-lgthread \
	$(NULL)

else
#No need to specify explicitly, configure should detect the
#needed compiler and linker flags.
#CFLAGS+=-DGTKMM_2

#GTK_INCLUDE=\
 	-I/usr/include/cairo\
	-I${GTKMM_PREFIX}/include/cairomm-1.0 \
 	-I${GLIB_PREFIX}/include/glib-2.0 \
 	-I${GLIB_PREFIX}/$(LIBDIR)/glib-2.0/include \
	-I${GTK_PREFIX}/include/gtk-2.0 \
 	-I${GTK_PREFIX}/$(LIBDIR)/gtk-2.0/include \
	-I${GTK_PREFIX}/include/atk-1.0 \
	-I${GTK_PREFIX}/include/pango-1.0 \
	-I${GTK_PREFIX}/include/pangomm-1.4 \
	-I${GTKMM_PREFIX}/$(LIBDIR)/glibmm-2.4/include \
	-I${GTKMM_PREFIX}/include/glibmm-2.4 \
	-I${GTKMM_PREFIX}/include/atkmm-1.6 \
	-I${GTKMM_PREFIX}/$(LIBDIR)/gdkmm-2.4/include \
	-I${GTKMM_PREFIX}/include/gdkmm-2.4 \
	-I${GTKMM_PREFIX}/$(LIBDIR)/gtkmm-2.4/include \
	-I${GTKMM_PREFIX}/include/gtkmm-2.4 \
	-I${SIGC_PREFIX}/$(LIBDIR)/sigc++-2.0/include \
	-I${SIGC_PREFIX}/include/sigc++-2.0 \
	-I/usr/include/libxml2 \
	-I/usr/include/libart-2.0 \
	-I${GLIB_PREFIX}/include/libgnomeprint-2.2 \
	$(NULL)

#EXTRA_LIBS=\
	-lgtkhtml-3.6			\
	-latk-1.0				\
	-lpango-1.0				\
	-lgconf-2				\
	-lgailutil				\
	-lgdk_pixbuf-2.0		\
	-lgnomeprint-2-2		\
	$(NULL)

#LDLIBS+=\
	-lgtkmm-2.4				\
	-lgdkmm-2.4				\
	-latkmm-1.6				\
	-lpangomm-1.4			\
	-lglibmm-2.4			\
	-lsigc-2.0				\
	-lgtk-x11-2.0			\
	-lgdk-x11-2.0			\
	-lgmodule-2.0			\
	-lglib-2.0				\
	-lgobject-2.0			\
	-lgthread-2.0			\
	$(NULL)

endif

LDLIBS+=-lXext -lXi -lX11 -ldl
LDFLAGS+=-L/usr/X11R6/$(LIBDIR) -L/usr/$(LIBDIR)

CXXFLAGS+=$(GTK_INCLUDE)

