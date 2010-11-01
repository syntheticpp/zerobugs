# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id: Makefile 588 2008-08-06 04:50:35Z root $
#
ifeq ($(TOP),)
 TOP=../..
endif

all: test
AUTOTEST=../../plugin/autotest/autotest.so
include $(TOP)/zdk/make/Common.mak

LIBS+=-ltypez -lzdk

test: AUTOTEST
	./test.sh

test32: AUTOTEST
	setarch i386 ./test_32on64.sh

#lib_unit_tests:
#	cd $(TOP)/zdk/src && $(MAKE) test
#	cd $(TOP)/dharma && $(MAKE) test
#	cd $(TOP)/unmangle && $(MAKE) test
#	cd $(TOP)/symbolz && $(MAKE) test

AUTOTEST:
	cd ../../plugin/autotest; $(MAKE)

include $(TOP)/zdk/make/Epilogue.mak

USER_CLEAN=clean_plugin
clean_plugin:
	cd ../../plugin/autotest; $(MAKE) clean

EXTRA_CLEAN=\
	testlog.* testsuite.log libfoo.so libbar.so *.zero *.debug \
	a.out foo bar a.cpp foo.cpp foo.h bar.cpp script auto.py core* \
	.zero testsuite.out fred .zero

