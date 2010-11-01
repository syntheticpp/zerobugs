# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 588 2008-08-06 04:50:35Z root $
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

LIBS=-lelfz -ldharma -lzdk
LDLIBS+=-ludis86 -lelf -lpthread

#ifneq ($(ARCH),x86_64)
# LDLIBS+=/lib/ld-linux.so.2

LDFLAGS+=-Wl,--no-undefined
#endif

SRCS=disasm.cpp 

PLUGIN=$(PLUGIN_PATH)zdisasm.so
TARGET=$(PLUGIN)

all: $(PLUGIN)

$(PLUGIN): $(OBJS) $(LIB_FILES)
	mkdir -p $(PLUGIN_PATH); \
	$(LINK) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)

include $(TOP)/zdk/make/Epilogue.mak
