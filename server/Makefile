# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

LIBS=-lrpc -ldharma -lzdk
LDLIBS+=-lpthread

################################################################
# sources
SRCS=\
	factory.cpp				\
	main.cpp 				\
	server_socket.cpp		\
	socket_base_server.cpp	\
	$(NULL)

################################################################

TARGET=$(BIN_PATH)zserver

all: $(TARGET)

$(TARGET): $(LIB_FILES) $(OBJS) $(BIN_PATH)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

include $(TOP)/zdk/make/Epilogue.mak
