# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile.in 707 2010-10-10 00:06:21Z root $
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

TARGET=$(BIN_PATH)zero


LIBS+=\
	-linterp \
	-lsymbolz \
	-ltarget \
	-ltypez \
	-ldharma \
	-lzdk \
	-lelfz \
	-lunmangle \
	$(NULL)

lib_unit_tests: $(LIB_FILES)
	cd $(TOP)/zdk/src && $(MAKE) test
	cd $(TOP)/dharma && $(MAKE) test
	cd $(TOP)/unmangle && $(MAKE) test
	cd $(TOP)/symbolz && $(MAKE) test

CXXFLAGS+=-I. -DUSER=\"$(USER)\" -DHOSTNAME=\"$(HOSTNAME)\" 

################################################################
# Architecture-dependent stuff
ifeq ($(ARCH), x86_64)
 ARCH_DEP_SRCS=					\
	cprologue64.cpp				\
 	stack-x86_64.cpp			\
 	$(NULL)
 FBSD_SRCS=						\
	$(NULL)
 LINUX_SRCS=					\
 	$(NULL)
else 
ifeq ($(ARCH), ppc)
 ARCH_DEP_SRCS=					\
 	stack-ppc.cpp				\
	$(NULL)
else
 ARCH_DEP_SRCS=					\
 	stack-i386.cpp				\
 	$(NULL)
 FBSD_SRCS=						\
 	$(NULL)
 LINUX_SRCS=					\
	$(NULL)
endif
endif

FBSD_SRCS+=						\
	$(NULL)

LINUX_SRCS+=					\
	$(NULL)

ifeq ($(UNAME), Linux)
	ARCH_DEP_SRCS+=$(LINUX_SRCS)
else
 ifeq ($(UNAME), FreeBSD)
	ARCH_DEP_SRCS+=$(FBSD_SRCS)
 endif
endif

################################################################
SRCS=$(ARCH_DEP_SRCS)			\
	breakpoint.cpp 				\
	breakpoint_img.cpp 			\
	breakpoint_grp.cpp 			\
	breakpoint_mgr.cpp 			\
	core_thread.cpp				\
	cprologue32.cpp				\
	debugger_base.cpp 			\
	debugger_engine.cpp 		\
	debugger_shell.cpp 			\
	deferred_breakpoint.cpp		\
	disassemble.cpp				\
	history.cpp					\
	init.cpp					\
	internal_cmd.cpp 			\
	module.cpp					\
	proc_service.cpp			\
	process.cpp					\
	read_line.cpp				\
	ret_symbol.cpp				\
	signal_policy.cpp 			\
	source.cpp					\
	step_over_mgr.cpp			\
	stack_trace.cpp 			\
	symbol_events.cpp 			\
	thread.cpp 					\
	thread_base.cpp				\
	thread_db_error.cpp			\
	update_impl.cpp				\
	unhandled_map.cpp			\
	watchpoint.cpp				\
	$(NULL)

# uncomment if you want to use GNU readline, and also
# remember to define the USE_GNU_READLINE preprocessor macro
#READLINE_LIB=-lreadline -lcurses

LDLIBS+=$(LIBELF) $(READLINE_LIB) -lthread_db 
LDLIBS+=-lpthread


################################################################
all: $(TARGET)

################################################################
SRCS+=main.cpp

debugger_shell.cpp: version.h sysid.h

$(TARGET): $(BIN_PATH) $(LIB_FILES) $(OBJS)
	$(LINK) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

#make sure that "make clean" removes version.h
EXTRA_CLEAN=version.h

version.h:
	echo "#define VERSION \"$(ARCH) `uname -r` `date +%m%d%y`\"" > $@

sysid.h:
	echo "#define SYSID \"`$(TOP)/make/sysid`\"" > $@

depend: sysid.h version.h
include $(TOP)/zdk/make/Epilogue.mak
