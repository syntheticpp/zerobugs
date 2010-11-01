# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 697 2010-09-20 08:20:52Z root $
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

ifeq ($(ENABLE_DYNAMIC_STABZ),)
 LIB=libstabz.a
else
 LIB=libstabz.so
 CXXFLAGS+=-O3 -DPROFILE -fPIC
endif

#ifneq ($(RELEASE),)
# CXXFLAGS+=-finline-functions 
#endif

TARGET=$(LIB_PATH)$(LIB)

CXXFLAGS+=-I. -I../elfz


################################################################
SRCS=\
	block.cpp \
	compile_unit.cpp \
	compile_unit_parse.cpp \
	datum.cpp \
	descriptor.cpp \
	descriptor_parse.cpp \
	function.cpp \
	fwdtype.cpp \
	init_events.cpp \
	object.cpp \
	parse.cpp \
	parse_state.cpp \
	util.cpp \
	variable.cpp \
	$(NULL)

#DEPSRCS=$(SRCS)
USER_CLEAN=clean_tests

################################################################

ifeq ($(ENABLE_DYNAMIC_STABZ),)
$(TARGET): $(OBJS) 
	mkdir -p $(LIB_PATH)
	$(AR) -r $@ $(OBJS)
else

# keep sprof happy...
LDLIBS+=-ltypez -ldharma -lelfz $(LIBELF) -lunmangle -lzdk

$(TARGET): $(OBJS) 
	mkdir -p $(LIB_PATH)
	$(LINK) $(OBJS) $(LDFLAGS) $(LDLIBS) -shared --Wl,-soname=$(LIB) -o $@
endif

all: $(TARGET)


test: $(TARGET)
	cd tests && $(MAKE); ./tests
clean_tests:
	cd tests && $(MAKE) clean

include $(TOP)/zdk/make/Epilogue.mak
