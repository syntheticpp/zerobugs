# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 588 2008-08-06 04:50:35Z root $
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

LIBS=-ldharma -lzdk

LDLIBS+=$(LIBS) -lpthread 
#LPLUGIN_PATH=$(TOP)/engine/testsuite/
LPLUGIN_PATH=.

SRCS=autotest.cpp command.cpp

PLUGIN=$(PLUGIN_PATH)autotest.so
TARGET=$(PLUGIN)

all: $(PLUGIN)

$(PLUGIN): $(LIB_FILES) $(OBJS)
	mkdir -p $(PLUGIN_PATH)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)

include $(TOP)/zdk/make/Epilogue.mak
