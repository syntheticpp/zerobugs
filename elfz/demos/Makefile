# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 697 2010-09-20 08:20:52Z root $
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

CXXFLAGS+=-I..

# additional libes for the demo programs
LDLIBS=			\
	-lsymbolz 	\
	-lunmangle	\
	-lelfz 		\
	-ldharma 	\
	-lzdk 		\
	$(LIBELF)	\
	-lpthread	\
	$(LIBDL)	\
	$(NULL)

ifneq ($(STL_LIB),)
 LDLIBS+=-l$(STL_LIB)
endif

################################################################
SRCS=\
	$(NULL)

DEPSRCS=$(SRCS) demo.cpp demo2.cpp readcore.cpp democore.cpp 

EXTRA_CLEAN=\
	demo.o demo \
	demo2.o demo2 \
	readcore readcore.o \
	democore.o democore  \
	listdep.o listdep


LIB=$(LIB_PATH)libelfz.a \
	$(LIB_PATH)libunmangle.a

################################################################
all: demo demo2 readcore democore listdep

demo: demo.o $(LIB)
	$(CXX) demo.o $(LDFLAGS) $(LDLIBS) -o $@

demo2: demo2.o $(LIB)
	$(CXX) demo2.o $(LDFLAGS) $(LDLIBS) -o $@

readcore: readcore.o $(LIB)
	$(CXX) readcore.o $(LDFLAGS) $(LDLIBS) -o $@

democore: democore.o $(LIB)
	$(CXX) democore.o $(LDFLAGS) $(LDLIBS) -o $@

listdep: listdep.o $(LIB)
	$(CXX) listdep.o $(LDFLAGS) $(LDLIBS) -o $@ 

#		-Wl,"-R/usr/local/lib;`pwd`:;/usr/local" -o $@

include $(TOP)/zdk/make/Epilogue.mak
