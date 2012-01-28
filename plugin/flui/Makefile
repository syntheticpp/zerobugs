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
SRCS=	flui.cpp 			\
		flmenu.cpp			\
		pack_layout.cpp		\
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
