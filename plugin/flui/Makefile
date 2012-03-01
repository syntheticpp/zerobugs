# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

# zero-specific libs
LIBS=-ldharma -lzdk

# other libs
LDLIBS+=-lfltk -lpthread

LDFLAGS+=-Wl,--no-undefined

################################################################
SRCS=	code_view.cpp		\
		controller.cpp		\
		dialog.cpp			\
		flcode_view.cpp		\
		flcode_table.cpp	\
		fldialog.cpp		\
		fleval_dlg.cpp		\
		flui.cpp 			\
		flmain_window.cpp	\
		flmenu.cpp			\
		flpack_layout.cpp	\
		flstack_view.cpp	\
		fltoolbar.cpp		\
		flvar_view.cpp		\
		flview.cpp			\
		locals_view.cpp		\
		menu.cpp			\
		var_view.cpp		\
		view.cpp			\
		stack_view.cpp		\
		symkey.cpp			\
		toolbar.cpp			\
		$(NULL)

PLUGIN=$(PLUGIN_PATH)zflui.so
TARGET=$(PLUGIN)

################################################################

all: $(PLUGIN)

$(PLUGIN): $(LIB_FILES) $(OBJS)
	mkdir -p $(PLUGIN_PATH)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)


include $(TOP)/zdk/make/Epilogue.mak
