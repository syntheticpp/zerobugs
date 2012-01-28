# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

LIBS=-lrpc -ldharma -lzdk
LDLIBS+=-lpthread

################################################################
# sources
SRCS=\
	main.cpp 				\
	$(NULL)

################################################################

TARGET=test-client

all: $(TARGET)

$(TARGET): $(LIB_FILES) $(OBJS) $(BIN_PATH)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

include $(TOP)/zdk/make/Epilogue.mak
