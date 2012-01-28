# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

LIBS+=-lrpc -ltarget -ltypez -lsymbolz -ldharma -lzdk
LDLIBS+=$(LIBELF) -lpthread

CFLAGS+=-I$(TOP)/zdk/include \
		-I$(TOP)/engine \
		$(UNDO_INCLUDES)

SRCS=\
	main.cpp \
	proxy-plugin.cpp \
	proxy-target.cpp \
	$(NULL)

TARGET=$(PLUGIN_PATH)zremote-proxy.so

all: $(TARGET) 

$(TARGET): $(OBJS) $(LIB_FILES)
	$(LINK) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)

include $(TOP)/zdk/make/Epilogue.mak

