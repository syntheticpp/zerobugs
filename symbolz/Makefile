# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 705 2010-09-29 11:03:05Z root $
#
ifeq ($(TOP),)
 TOP=..
endif

LIB=libsymbolz.a
include $(TOP)/zdk/make/Common.mak

TARGET=$(LIB_PATH)$(LIB)
CXXFLAGS+=-I. -I../elfz

SRCS=\
	link_data.cpp \
	offset_symbol.cpp \
	symbol.cpp \
	symbol_map.cpp \
	symbol_table.cpp \
	symbol_table_events.cpp \
	$(NULL)

$(TARGET): $(LIB_PATH) $(OBJS)
	$(AR) -r $@ $(OBJS)

all: test

test: $(TARGET)
	cd tests; $(MAKE); ./tests

test_clean:
	cd tests; $(MAKE) clean

USER_CLEAN=test_clean

include $(TOP)/zdk/make/Epilogue.mak
