# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

#CXXFLAGS+=-fPIC

LIBS=-lstabz -ltypez -lelfz -ldharma -lunmangle -lzdk

LDLIBS+=$(LIBELF) -pthread
LDFLAGS+=-Wl,-soname=zstabs.so 


################################################################
SRCS=reader.cpp

PLUGIN=$(PLUGIN_PATH)zstabs.so
TARGET=$(PLUGIN)

################################################################

all: $(PLUGIN)

$(PLUGIN): $(OBJS) $(LIB_FILES)
	mkdir -p $(PLUGIN_PATH);
	$(LINK) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)


include $(TOP)/zdk/make/Epilogue.mak
