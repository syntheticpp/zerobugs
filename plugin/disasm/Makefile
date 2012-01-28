# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

LIBS=-ldharma -lzdk
LDLIBS+=-lpthread

LDFLAGS+=-Wl,--no-undefined

SRCS=disasm.cpp 

PLUGIN=$(PLUGIN_PATH)zdisasm.so
TARGET=$(PLUGIN)

all: $(PLUGIN)

$(PLUGIN): $(OBJS) $(LIB_FILES)
	mkdir -p $(PLUGIN_PATH); \
	$(LINK) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)

include $(TOP)/zdk/make/Epilogue.mak
