# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=..
endif
include $(TOP)/zdk/make/Common.mak
CC=gcc
CFLAGS+=-I.

ifneq ($(RELEASE),)
DEBUG_FORMAT=-g
CXXFLAGS+=				\
	-O3					\
	-finline-functions	\
	-funroll-loops		\
	-funroll-all-loops	\
	-fstrength-reduce	\
	-fomit-frame-pointer\
	-DNDEBUG
endif

LIBS=
LDLIBS=$(LIBS) $(LIBDL) -lpthread

ifneq ($(STL_LIB),)
 LDLIBS+=-l$(STL_LIB)
endif
TARGET=$(LIB_PATH)libunmangle.a

SRCS=encoder.cpp unmangle.cpp

all: $(TARGET) test

$(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH)
	$(AR) -r $@ $(OBJS)

# Tests
EXTRA_OBJ=main.o 	\
	test_decoder.o 	\
	test_legacy.o	\
	$(NULL)
EXTRA_CLEAN=$(EXTRA_OBJ) cxxfilt *.o \
	test_decoder 	\
	test_legacy		\
	$(NULL)

EXTRA_DEP=$(EXTRA_OBJ:%.o=%.cpp)
DEPSRCS+=$(EXTRA_DEP)

test: test_decoder test_legacy
	./test_decoder
	./test_legacy

ifneq ($(CXX),g++)
test_decoder: test_decoder.o $(LIB_FILES)
	$(CXX) $< $(LDFLAGS) $(LDLIBS) -L/usr/local/lib -o $@

test_legacy: test_legacy.o $(LIB_FILES)
	$(CXX) $< $(LDFLAGS) $(LDLIBS) -L/usr/local/lib -o $@
	
cxxfilt: main.o $(OBJS) $(LIB_FILES)
	$(CXX) main.o $(LDFLAGS) $(LDLIBS) \
		-L/usr/local/lib -liberty -o $@
else
test_decoder: test_decoder.o $(LIB_FILES)
	$(LINK) $< $(LDFLAGS) $(LDLIBS) -lunmangle -o $@
	
test_legacy: test_legacy.o $(LIB_FILES)
	$(LINK) $< $(LDFLAGS) $(LDLIBS) -o $@
	
cxxfilt: main.o $(OBJS) $(LIB_FILES)
	$(CXX) main.o -lunmangle $(LDFLAGS) $(LDLIBS) -L/usr/local/lib -o $@
endif

include $(TOP)/zdk/make/Epilogue.mak
