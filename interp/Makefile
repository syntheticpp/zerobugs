# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak

YACC=bison
LEX=flex
YFLAGS=-d 
ifeq ($(CXX),g++)
 CXXFLAGS+=-Wno-deprecated 
endif
CFLAGS+=-I$(TOP)/engine/

LIBS=-ltypez -lunmangle -ldharma -lzdk

LDLIBS=$(LIBS) $(LIBDL) -lpthread 
ifneq ($(STL_LIB),)
 LDLIBS+=-l$(STL_LIB)
endif

TARGET=$(LIB_PATH)libinterp.a

################################################################
SRCS= 						\
	abstract_decl.cpp		\
	additive_expr.cpp		\
	assign.cpp				\
	binary_expr.cpp			\
	bit_expr.cpp			\
	call_setup.cpp			\
	cast_expr.cpp			\
	collect_symbol.cpp		\
	conditional_expr.cpp	\
	constant.cpp			\
	context.cpp				\
	context_impl.cpp		\
	cpu_state_saver.cpp		\
	expr.cpp				\
	eval_inc_dec.cpp		\
	eval_sym.cpp			\
	frame_setup.cpp			\
	grammar.cpp				\
	ident.cpp				\
	interp.cpp				\
	lexer.cpp				\
	logical_expr.cpp		\
	lookup_methods.cpp		\
	multiplicative_expr.cpp	\
	on_call_return.cpp		\
	parameter_list.cpp		\
	postfix_expr.cpp		\
	primary_expr.cpp		\
	qualifier_list.cpp		\
	relational_expr.cpp		\
	slicer.cpp				\
	type_spec.cpp			\
	unary_expr.cpp			\
	variant_convert.cpp		\
	wrapscan.cpp			\
	$(NULL)

DEPSRCS+=					\
	main.cpp				\
	test_context.cpp		\
	test_debug_symbol.cpp	\
	../engine/ret_symbol.cpp\
	$(NULL)


scan.cpp: scan.l
	$(LEX) $(FLEXFLAGS) -o$@ $<

# workaround bug in Flex 2.5
wrapscan.cpp: scan.cpp

grammar.cpp: grammar.y
	$(YACC) $(YFLAGS) -o $@ $<
	-mv grammar.cpp.h grammar.h
	-mv grammar.hpp grammar.h

relational_expr.o: relational_expr.cpp
	$(CXX) $(CXXFLAGS) -Wno-sign-compare -c $<

EXTRA_CLEAN=$(TESTOBJ) test-interp grammar.output scan.cpp

TESTOBJ=main.o test_context.o test_debug_symbol.o ../engine/ret_symbol.o

################################################################

all: $(TARGET) test

$(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH)
	$(AR) -r $@ $(OBJS)

test-interp: $(OBJS) $(TESTOBJ) $(LIB_FILES)
	$(LINK) $(OBJS) $(TESTOBJ) $(LDFLAGS) \
	-Wl,-start-group $(LDLIBS) -Wl,-end-group -o $@ 

test: test-interp
	./test-interp

depend: scan.cpp
include $(TOP)/zdk/make/Epilogue.mak
