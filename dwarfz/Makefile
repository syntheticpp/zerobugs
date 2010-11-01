# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 715 2010-10-17 21:43:59Z root $
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

TARGET=$(LIB_PATH)libdwarfz.a
CXXFLAGS+=-frtti -Iprivate -Ipublic


ifneq ($(RELEASE),)
# 2.96 used to randomly crashes with -O2 and -fsjlj-exceptions
 ifeq ($(CXXVER),2.96)
  CXXFLAGS+=-O
 endif
endif


LIBS=-lzdk -ldharma -lsymbolz

# additional libes for the sample program:
LDLIBS=-ldwarfz -ldwarf $(LIBELF) -lpthread

################################################################
SRCS=\
	abi_x86.cpp \
	access_attr.cpp \
	aggregation.cpp \
	array_type.cpp \
	attr.cpp \
	base_type.cpp \
	block.cpp \
	class_type.cpp \
	compile_unit.cpp \
	comp_dir_attr.cpp \
	const_type.cpp \
	const_value.cpp \
	datum.cpp \
	debug.cpp \
	decorated_type.cpp \
	delegate.cpp \
	die.cpp \
	encoding_attr.cpp \
	enum_type.cpp \
	error.cpp \
	factory.cpp \
	function.cpp \
	globals.cpp \
	imported_decl.cpp \
	inheritance.cpp \
	init.cpp \
	inlined_instance.cpp \
	location.cpp \
	location_attr.cpp \
	log.cpp \
	member.cpp \
	namespace.cpp \
	pointer_type.cpp \
	producer.cpp \
	ptr_to_member_type.cpp \
	subrange_type.cpp \
	subroutine_type.cpp \
	type.cpp \
	type_attr.cpp \
	typedef.cpp \
	unwind.cpp \
	utils.cpp \
	variable.cpp \
	volatile_type.cpp \
	$(NULL)

DEPSRCS=$(SRCS) main.cpp

################################################################

TEST_OBJS=testfwd.o test.o
EXTRA_CLEAN=test $(TEST_OBJS)

all: $(TARGET)

sample: main.o $(TARGET)
	$(CXX) main.o $(LDFLAGS) $(LDLIBS) -o $@

test: $(TARGET) $(TEST_OBJS) $(LIB_FILES)
	$(CXX) $(TEST_OBJS) $(LDFLAGS) $(LDLIBS) $(LIBS) -o $@

$(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH); $(AR) -r $@ $(OBJS);

include $(TOP)/zdk/make/Epilogue.mak
