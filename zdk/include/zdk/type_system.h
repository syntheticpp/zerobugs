#ifndef TYPE_SYSTEM_H__9A4BC9D9_B003_491B_8099_BF4F5441F7E5
#define TYPE_SYSTEM_H__9A4BC9D9_B003_491B_8099_BF4F5441F7E5
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/shared_string.h"
#include "zdk/string_cache.h"
#include "zdk/types.h"

struct DebugInfoReader;
struct DebugSymbol;


/**
 * Interface to the type system. Objects representing data
 * types are created on an as-needed basis.
 */
DECLARE_ZDK_INTERFACE_(TypeSystem, StringCache)
{
    DECLARE_UUID("e180f8e0-7717-40f3-abc7-0ad35eb68cd8")

    /**
     * @return a pointer to the object that represents the
     * C/C++ "void" type
     */
    virtual DataType* get_void_type() = 0;

    /**
     * @return a pointer to an object that represents an
     * integer type, of the specified size (in bits);
     * @note the size is specified in bits, so that we can
     * model bit-fields in C/C++ structs and classes.
     * @param name specifies the name of the type
     * @note this parameter is used only when the type is
     * first created; if a type of the given bit width and
     * signed-ness exists, then that type object is returned.
     * @param bitSize the size of the type, in bits, including
     * the sign bit
     * @param isSigned if true, the returned object represents
     * a signed integer type.
     * Example:
     * @code
     *  get_int_type(new SharedStringImpl("long"), 32, true);
     * @endcode
     */
    virtual DataType* get_int_type(SharedString*   name,
                                   bitsize_t       bitSize,
                                   bool            isSigned) = 0;

    /**
     * @return a pointer to an object that represents a floating
     * point type, of the specified width.
     * @note the size is given in bytes, since, unlike for ints,
     * bit-fields of floating type are not allowed.
     */
    virtual DataType* get_float_type(
        SharedString*   name,
        size_t          byteSize) = 0;

    virtual DataType* get_bool_type(size_t nbits) = 0;

    virtual DataType* get_array_type(int64_t rangeLow,
                                     int64_t rangeHigh,
                                     DataType* elemType) = 0;

    virtual ClassType* get_class_type(const char* name,
                                     size_t bitSize,
                                     bool isUnion = false) = 0;

    virtual ClassType* get_unnamed_class_type(size_t bitSize,
                                     bool isUnion = false) = 0;

    /**
     * Given a data type, return the object that represents a pointer
     * to that type. For example, if pointedType represents a 32-bit,
     * signed integer, the returned object represents type int* type.
     */
    virtual PointerType* get_pointer_type(DataType* pointedType) = 0;

    /**
     * Given a data type, return the object that represents a reference
     * to that type. For example, if type represents a 32-bit,
     * signed integer, the returned object represents type int& type.
     */
    virtual DataType* get_reference_type(DataType* type) = 0;

    /**
     * Given a data type, return an object representing the qualified
     * type. For example, if the data type is int and the qualifier is
     * QUALIFIER_CONST, the returned object will represent the
     * "const int" type.
     */
    virtual DataType* get_qualified_type(DataType*, Qualifier) = 0;

    /**
     * @return an object that represents a function type.
     * @param retType pointer to the object representing the return type
     * @note NULL may be passed in for function returning void
     * @param argTypes pointer to an array of types; the argument types;
     *  must contain argCount elements. May be NULL if argCount is zero.
     * @param argCount the number of arguments that the function takes
     * @param vargArgs true if the function has variable arguments
     */
    virtual FunType* get_fun_type(DataType*         retType,
                                  DataType* const*  argTypes,
                                  size_t            argCount,
                                  bool              varArgs,
                                  bool              strict = true) = 0;

    /**
     * Return a pseudo-type so that we can evaluate macros as
     * constant strings (what the macro expands to).
     */
    virtual DataType* get_macro_type() = 0;

    virtual DataType* get_string_type() = 0;
    virtual DataType* get_wide_string_type() = 0;

    //virtual SharedString* get_string(const char*, size_t len = 0) = 0;
    //virtual SharedString* get_shared_string(SharedString*) = 0;

    /**
     * @return true if the RunTime Type Info (rtti) should be
     * auto-detected; if true, the builtin intepreter
     * dereferences pointer-to-base expressions as if they
     * were pointer-to-derived.
     */
    virtual bool use_auto_rtti() const = 0;

    virtual void set_auto_rtti(bool) = 0;

    /**
     * own the memory of the passed DataType object
     */
    virtual DataType* manage(DataType*) = 0;

    /**
     * @return the size in bits for the debugged target
     */
    virtual size_t word_size() const = 0;

    virtual DebugSymbol* create_debug_symbol(
        DebugInfoReader*,
        Thread*,
        DataType*,
        SharedString*,
        addr_t,
        SharedString* declFile = NULL,
        size_t declLine = 0,
        bool isReturnValue = false) const = 0;

    virtual DataType* get_complex_float(SharedString*, size_t byteSize) = 0;
    virtual DataType* get_complex_int(SharedString*, size_t byteSize) = 0;

    virtual DataType* get_ptr_to_member_type(DataType*, DataType*) = 0;
};

#endif // TYPE_SYSTEM_H__9A4BC9D9_B003_491B_8099_BF4F5441F7E5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
