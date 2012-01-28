# vim: noexpandtab: tabstop=4: shiftwidth=4
# $Id$
#
ifeq ($(TOP),)
 TOP=..
endif

include $(TOP)/zdk/make/Common.mak
#ENABLE_DYNAMIC_TYPEZ=1

ifeq ($(ENABLE_DYNAMIC_TYPEZ),)
 TARGET=$(LIB_PATH)libtypez.a
else
 TARGET=$(LIB_PATH)libtypez.so
endif

################################################################
SRCS=						\
	adjust_base.cpp			\
	complex.cpp				\
	data_type_impl.cpp 		\
	debug_symbol_impl.cpp	\
	debug_symbol_array.cpp	\
	debug_symbol_vector.cpp	\
	is_cv_qualified.cpp		\
	lookup_child.cpp		\
	native_type_system.cpp	\
	remove_qual.cpp			\
	rtti.cpp				\
	string_pool.cpp			\
	template_param.cpp		\
	types.cpp 				\
	$(NULL)


################################################################

all: $(TARGET)

ifeq ($(ENABLE_DYNAMIC_TYPEZ),)
$(TARGET): $(OBJS) 
	mkdir -p $(LIB_PATH);
	$(AR) -r $@ $(OBJS);
else

# keep sprof happy...
 LDLIBS+=-ldharma -lelfz $(LIBELF)-lunmangle -lzdk

 $(TARGET): $(OBJS)
	mkdir -p $(LIB_PATH)
	$(LINK) $(OBJS) $(LDFLAGS) $(LDLIBS) -shared --Wl,-soname=libtypez.so -o $@
endif

include $(TOP)/zdk/make/Epilogue.mak
