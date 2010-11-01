# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 705 2010-09-29 11:03:05Z root $
#
LIB=

ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

TARGET=tests

LIBS=-lstabz \
	-ltypez \
	-lunmangle \
	-lelfz \
	-lzdk \
	-ldharma

LDLIBS:=-lsymbolz $(LIBS) $(LDLIBS) $(LIBELF) -lpthread

CXXFLAGS+=-I.. -Wno-write-strings
#CFLAGS+=-gstabs+
CFLAGS+=-g

#CFLAGS+=-pg
#LDFLAGS+=-pg 

################################################################
SRCS=\
	tests.cpp \
	$(NULL)

EXTRA_CLEAN=tests.cache.*

################################################################

all: $(TARGET)

tests: $(LIB_FILES) tests.o
	$(CXX) tests.o $(LDFLAGS) $(LDLIBS) -o $@;


include $(TOP)/zdk/make/Epilogue.mak
