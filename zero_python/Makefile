# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak
include $(TOP)/zdk/make/Python.mak

TARGET=$(LIB_PATH)libzero_python.a
LIBS=
LDLIBS=
CXXFLAGS+=-Wno-write-strings

#boost python does not compile with optimizations ON with gcc 3.2
ifneq ($(CXX_MAJOR),4)
 OPTIMIZE=
endif


################################################################
SRCS=\
	breakpoint.cpp		\
	breakpoint_registry.cpp	\
	class_type.cpp		\
	data_type.cpp		\
	debug_event.cpp 	\
	debug_symbol.cpp 	\
	debug_sym_wrap.cpp 	\
	debugger.cpp		\
	dump_settings.cpp	\
	environ.cpp			\
	elf_header.cpp		\
	handle_error.cpp	\
	heap.cpp			\
	macro.cpp			\
	marshaller.cpp		\
	module.cpp			\
	process.cpp			\
	properties.cpp		\
	python_embed.cpp 	\
	python_mutex.cpp 	\
	symbol.cpp			\
	symbol_map.cpp		\
	symbol_table.cpp	\
	stack_trace.cpp		\
	thread.cpp			\
	translation_unit.cpp\
	type_system.cpp		\
	update.cpp			\
	utility.cpp			\
	$(NULL)

################################################################

TEST_OBJS=
EXTRA_CLEAN=$(TEST_OBJS)

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH); $(AR) -r $@ $(OBJS);

include $(TOP)/zdk/make/Epilogue.mak
