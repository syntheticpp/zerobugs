# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 704 2010-09-27 16:36:01Z root $
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

TARGET=readtest
CXXFLAGS+=

LIBS=-ldharma 
#DEBUG_FORMAT=-gstabs+
# additional libes for the sample program:
#LDLIBS=-lcurses 
#LDLIBS=-lstlport_gcc
#LDLIBS+=-ltermcap

################################################################
SRCS=\
	read_line.cpp \
	read_line_test.cpp \
	terminal.cpp \
	$(NULL)

DEPSRCS=$(SRCS) 

################################################################

$(TARGET): $(OBJS) $(LIB_FILES)
	$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) $(LDLIBS) -o $@

#$(OBJS): Makefile depend.make
#$(OBJS): depend.make
all: $(TARGET)

-include depend.make

include $(TOP)/zdk/make/Epilogue.mak

