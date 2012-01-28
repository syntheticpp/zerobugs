# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

TARGET=$(LIB_PATH)librpc.a


################################################################
SRCS=\
	dispatcher.cpp \
	msg.cpp \
	msgcli.cpp \
	msgsrv.cpp \
	remote_ifstream.cpp \
	remote_io.cpp \
	$(NULL)

################################################################

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH); 
	$(AR) -r $@ $(OBJS);

include $(TOP)/zdk/make/Epilogue.mak
