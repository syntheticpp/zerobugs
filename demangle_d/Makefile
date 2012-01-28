# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
# Makefile, Copyright (c) 2006 Cristian Vlasceanu, http://www.zero-bugs.com
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

CFLAGS=-g -fPIC -D_GNU_SOURCE

OBJS=$(SRCS:%.c=%.o)

#Zero's memory model is multi-threaded
LDLIBS+=$(LIBS) -lpthread

SRCS=demangle.c

TARGET=$(LIB_PATH)libdemangle_d.so

all: $(TARGET) 

$(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH)
	$(LINK) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)


include $(TOP)/zdk/make/Epilogue.mak

