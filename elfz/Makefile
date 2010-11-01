# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 588 2008-08-06 04:50:35Z root $
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

TARGET=$(LIB_PATH)libelfz.a
#TARGET=$(LIB_PATH)libelfz.so

CXXFLAGS+=-I.

#ifneq ($(CXX_MAJOR),2)
# ifneq ($(CXX_MAJOR),3)
#   ifneq ($(ARCH),x86_64)
# 	CXXFLAGS+=-fvisibility=hidden
#   endif
# endif
#endif

LDLIBS=

################################################################
SRCS=\
	binary.cpp \
	core_file.cpp \
	dyn_lib_list.cpp \
	error.cpp \
	file.cpp \
	headers.cpp \
	ldsoconf.cpp \
	note.cpp \
	program_header.cpp \
	section.cpp \
	selector.cpp \
	symbol_table.cpp \
	$(NULL)

#DEPSRCS=$(SRCS)

################################################################

$(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH); 
	$(AR) -r $@ $(OBJS);

all: $(TARGET) 

#EXTRA_CLEAN=

include $(TOP)/zdk/make/Epilogue.mak
