# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=../..
endif
include $(TOP)/zdk/make/Common.mak

LIBS=-ldwarfz -ltypez -ldharma -lunmangle -lelfz -lzdk 
LDLIBS+=-ldwarf $(LIBELF) -lpthread

LDFLAGS+=-Wl,--no-undefined

CXXFLAGS+=-frtti -I.

ifneq ($(RELEASE),)
# 2.96 randomly crashes with -O2 and -fsjlj-exceptions
 ifeq ($(CXXVER),2.96)
  CXXFLAGS+=-O
 endif
endif
################################################################
SRCS=						\
	addr_operations.cpp		\
	assoc_array.cpp			\
	class_type_proxy.cpp	\
	const_value.cpp			\
	debug_cache.cpp 		\
	dynamic_array.cpp		\
	frame.cpp				\
	reader.cpp 				\
	type_adapter.cpp 		\
	type_map.cpp			\
	unit.cpp				\
	$(NULL)

PLUGIN=$(PLUGIN_PATH)zdwarf.so
TARGET=$(PLUGIN)

################################################################
all: $(TARGET)

$(PLUGIN): $(LIB_FILES) $(OBJS)
	mkdir -p $(PLUGIN_PATH);
	$(LINK) $(LDFLAGS) -shared $(OBJS) -o $@ $(LDLIBS)

include $(TOP)/zdk/make/Epilogue.mak

