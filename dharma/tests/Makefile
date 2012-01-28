# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=../..
endif

include $(TOP)/zdk/make/Common.mak

CXXFLAGS+=-ftemplate-depth-32 
CC=$(CXX)
LIBS=-lzdk

LDLIBS+=-ldharma $(LIBS) -ldl -lpthread

################################################################
SRCS=\
	test_auto_handle.cpp \
	test_dir.cpp	\
	test_dynlib.cpp	\
	test_fstream.cpp \
	test_process_name.cpp \
	test_ref_count.cpp \
	test_regs.cpp \
	test_system_error.cpp \
	test_task_pool.cpp \
	$(NULL)

################################################################

PROGS=$(SRCS:%.cpp=%) 

all: $(PROGS)
	for i in $(PROGS); do echo $$i; ./$$i; done

$(PROGS): $(LIB_FILES)
EXTRA_CLEAN=$(PROGS)

include $(TOP)/zdk/make/Epilogue.mak
