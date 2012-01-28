# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

TARGET=$(LIB_PATH)libdharma.a
TEST=test
USER_CLEAN=test_clean

SRCS=\
	canonical_path_unix.cpp \
	bstream.cpp \
	cstream.cpp \
	directory.cpp \
	dynamic_lib.cpp \
	dynamic_lib_unix.cpp \
	environ.cpp \
	exec.cpp \
	exec_arg.cpp \
	fstream.cpp \
	object_manager.cpp \
	redirect.cpp \
	pipe.cpp \
	plugin_manager.cpp \
	process_name_unix.cpp \
	properties.cpp \
	settings.cpp \
	sigutil.cpp \
	symbol_util.cpp \
	system_error.cpp \
	syscall_wrap.cpp \
	stream.cpp \
	switchable_action.cpp \
	task_pool.cpp \
	utility.cpp \
	variant.cpp \
	virtual_dso.cpp \
	$(NULL)

ifeq ($(UNAME),Darwin)
 SRCS+=syscall_mach.cpp
else
 SRCS+=syscall_unix.cpp
endif

ifeq ($(UNAME),FreeBSD)
 SRCS+=process_name_bsd.cpp
else
 SRCS+=process_name_linux.cpp
endif

all: $(TEST)

$(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH); 
	$(AR) -r $@ $(OBJS);

$(TEST): $(TARGET)
	cd tests; $(MAKE)

include $(TOP)/zdk/make/Epilogue.mak

test_clean:
	cd tests; $(MAKE) tidy
