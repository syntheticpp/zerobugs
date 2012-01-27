# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile.tmpl 588 2008-08-06 04:50:35Z root $
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
SRCS=	flui.cpp 			\
		flmenu.cpp			\
		menu.cpp			\
		user_interface.cpp	\
		$(NULL)

PLUGIN=$(PLUGIN_PATH)zflui.so
TARGET=$(PLUGIN)

################################################################

all: $(PLUGIN)

$(PLUGIN): $(LIB_FILES) $(OBJS)
	mkdir -p $(PLUGIN_PATH)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)


include $(TOP)/zdk/make/Epilogue.mak
