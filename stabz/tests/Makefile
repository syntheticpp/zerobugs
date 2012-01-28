# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
LIB=

ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

TARGET=tests

LIBS=-lstabz -ltypez -lelfz -lzdk -ldharma -lunmangle

LDLIBS:=$(LIBS) $(LDLIBS) $(LIBELF) -lpthread

CXXFLAGS+=-I. 
CFLAGS+=-gstabs+
#CFLAGS+=-pg
#LDFLAGS+=-pg 

################################################################
SRCS=\
	tests.cpp \
	builtin.cpp \
	typedef.cpp \
	a.cpp \
	b.cpp \
	$(NULL)

DEPSRCS=$(SRCS) 
EXTRA_CLEAN=abfwd builtin typedef 

################################################################

all: $(TARGET)

samples:
	$(CXX) -gstabs+ builtin.cpp -o builtin
	$(CXX) -gstabs+ typedef.cpp -o typedef
	$(CXX) -g a.cpp b.cpp -o abfwd
	
tests: $(LIB_FILES) samples tests.o
	$(CXX) tests.o $(LDFLAGS) $(LDLIBS) -o $@;


include $(TOP)/zdk/make/Epilogue.mak
