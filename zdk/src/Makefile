# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 704 2010-09-27 16:36:01Z root $
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

TARGET=$(LIB_PATH)libzdk.a
SHARED_LIBS=
CXXFLAGS+=-O2

################################################################
SRCS=\
	breakpoint_util.cpp \
	buffer.cpp 			\
	mutex.cpp 			\
	persistent.cpp		\
	shared_string.cpp 	\
	string.cpp			\
	thread_current.cpp 	\
	thread_frame_base.cpp \
	thread_util.cpp 	\
	traced.cpp			\
	uuid.cpp 			\
	uuid2.cpp 			\
	variant_print.cpp	\
	variant_true.cpp	\
	variant_type.cpp	\
	xtrace.cpp			\
	$(NULL)


################################################################

all: $(TARGET) test

$(TARGET): $(OBJS) 
	mkdir -p $(LIB_PATH); rm -f $@; $(AR) -r $@ $(OBJS)

################################################################
#
# TESTS
#
test_interface_cast: test_interface_cast.cpp
	$(CXX) $(CXXFLAGS) $< -L$(LIB_PATH) -lzdk -lpthread -o $@

test_ref_ptr: test_ref_ptr.cpp
	$(CXX) $(CXXFLAGS) $< -L$(LIB_PATH) -lzdk -lpthread -o $@

test_shared_string: test_shared_string.cpp
	$(CXX) $(CXXFLAGS) $< -L$(LIB_PATH) -lzdk -lpthread -o $@

teststr: teststr.cpp
	$(CXX) $(CXXFLAGS) $< -L$(LIB_PATH) -lzdk -lpthread -o $@

test_uuid: test_uuid.cpp
	$(CXX) $(CXXFLAGS) $< -L$(LIB_PATH) -lzdk -lpthread -o $@

#test_falloc: test_falloc.cpp
#	$(CXX) $(CXXFLAGS) $< -L$(LIB_PATH) -lzdk -lpthread -o $@

TESTS=\
	test_interface_cast\
	test_ref_ptr\
	test_shared_string\
	teststr\
	test_uuid\
	$(NULL)

EXTRA_CLEAN=$(TESTS)

test: $(TESTS)
	for i in $(TESTS); do echo $$i; ./$$i; done
################################################################


include $(TOP)/zdk/make/Epilogue.mak
