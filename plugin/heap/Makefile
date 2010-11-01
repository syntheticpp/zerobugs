# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 588 2008-08-06 04:50:35Z root $
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

LIBS=-ldharma -lzdk

LDLIBS+=$(LIBS) -lpthread 

#LDLIBS+=/lib/ld-linux.so.2
#LDFLAGS+=-Wl,--no-undefined


################################################################
SRCS= \
	cmd.cpp \
	heap.cpp \
	block.cpp \
	$(NULL)

PLUGIN=$(PLUGIN_PATH)zheap.so
TARGET=$(PLUGIN)

################################################################

all: $(PLUGIN)

$(PLUGIN): $(LIB_FILES) $(OBJS)
	mkdir -p $(PLUGIN_PATH); \
	$(LINK) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)


include $(TOP)/zdk/make/Epilogue.mak
