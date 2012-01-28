# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

CFLAGS+=-I$(TOP)/engine

ifneq ($(STL_LIB),)
 LDLIBS+=-l$(STL_LIB)
endif

TARGET=$(LIB_PATH)libtarget.a

# Architecture-dependent stuff
ifeq ($(ARCH), ppc)
LINUX_SRCS=				\
	debug_regs_ppc.cpp	\
	jump-ppc.cpp 	   	\
	linux_trampoline-ppc.cpp \
	linux-ppc.cpp		\
	$(NULL)
else 
ifeq ($(ARCH), x86_64)
 ARCH_DEP_SRCS=			\
 	$(NULL)
 FBSD_SRCS=				\
 	debug_regs_386.cpp	\
	fbsd-x86_64.cpp		\
	jump-x86.cpp 	   	\
	trampoline.cpp		\
	$(NULL)
 LINUX_SRCS=			\
 	debug_regs_386.cpp	\
	flags-i386.cpp		\
	jump-x86.cpp 	   	\
	linux-x86.cpp		\
	linux-x86_64.cpp	\
	linux_trampoline-x86_64.cpp \
 	$(NULL)
else
 ARCH_DEP_SRCS=			\
 	$(NULL)
 FBSD_SRCS=				\
 	debug_regs_386.cpp	\
	jump-x86.cpp 	   	\
	fbsd-i386.cpp		\
	flags-i386.cpp		\
	trampoline.cpp		\
 	$(NULL)
 LINUX_SRCS=			\
 	debug_regs_386.cpp	\
	flags-i386.cpp		\
	jump-x86.cpp 	   	\
	linux-x86.cpp		\
	linux-i386.cpp		\
	linux_trampoline-i386.cpp \
	$(NULL)
endif
endif

FBSD_SRCS+=				\
	bsd.cpp				\
	fbsd.cpp			\
	fbsd_core.cpp		\
	fbsd_live.cpp		\
	$(NULL)


LINUX_SRCS+=			\
	linux.cpp			\
	linux_core.cpp		\
	linux_live.cpp		\
	linux_live_xevent.cpp\
	$(NULL)

ifeq ($(UNAME), Linux)
	ARCH_DEP_SRCS+=$(LINUX_SRCS)
else
 ifeq ($(UNAME), FreeBSD)
	ARCH_DEP_SRCS+=$(FBSD_SRCS)
 endif
endif

################################################################
SRCS=$(ARCH_DEP_SRCS)	\
	syscall_pending.cpp \
	target.cpp			\
	target_factory.cpp	\
	target_manager.cpp	\
	unix.cpp			\
	$(NULL)

################################################################

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH)
	$(AR) -r $@ $(OBJS)


include $(TOP)/zdk/make/Epilogue.mak

-include depend.make
