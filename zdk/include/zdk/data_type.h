#ifndef DATATYPE_H__4D27E9C4_3B4A_40B9_A302_88262FC5AA44
#define DATATYPE_H__4D27E9C4_3B4A_40B9_A302_88262FC5AA44
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

#include "zdk/debug_sym.h"
#include "zdk/enum.h"
#include "zdk/observer.h"
#include "zdk/platform.h"
#include "zdk/ref_ptr_enforcer.h"
#include "zdk/shared_string.h"
#include "zdk/zobject.h"

class Buffer;
template<typename T> class WeakPtr;


/**
 * Base for abstract representation of data types
 */
DECLARE_ZDK_INTERFACE_(DataType, ZObject)
{
protected:
    virtual ~DataType() = 0;

public:
    DECLARE_UUID("7cb09f19-8766-4d14-ab85-a0edbc4bfaf9")

BEGIN_INTERFACE_MAP(DataType)
    INTERFACE_ENTRY(DataType)
END_INTERFACE_MAP()

    /**
     * The name of the datatype
     */
    virtual SharedString* name() const = 0;

    /**
     * The size of the data type, in bytes
     */
    virtual size_t size() const = 0;

    /**
     * The size of the data type, in bits
     */
    virtual size_t bit_size() const = 0;

    virtual bool is_fundamental() const = 0;

    virtual bool is_equal(const DataType*) const = 0;

    /**
     * Convert internally the two strings to the type that
     * the instance of the class represents; then, the two
     * values are compared.
     * @return -1 if the left-hand-side operand is less than
     * the right hand-side; 1 if greater, and zero if the two
     * values are equal.
     * @note The specifics of the comparison (less-than) depend on
     * the actual data types.
     * @note The two strings must not be NULL
     */
    virtual int compare(const char*, const char*) const = 0;

    /**
     * Examine the input string and determine if it contains a
     * valid representation of a value of the type the instance
     * of this class represents.
     * @return the number of characters up to where the string
     * contains a valid representation; for example, if this
     * instance corresponds to the `int' type, and the input
     * string is "123foobar", the method will return 3; if the
     * input is "0x123foobar", the method will return 6.
     */
    virtual size_t parse(const char*, Unknown2* = NULL) const = 0;

    /**
     * Writes a description of this data type to file descriptor;
     * @note mostly for debugging and testing purposes
     */
    virtual void describe(int fd) const = 0;

    /**
     * This function constructs the name of a pointer
     * or reference type to this type. For example, if
     * this type name is `A', and ptr is `*', the returned
     * string is `A*'
     * @note This level of indirection comes in handy for
     * types where constructing the pointer type name is not
     * as trivial as appending a star. For example, consider
     * a function type int(char); the pointer type name is
     * int (*)(char).
     */
    virtual SharedString* make_pointer_name(
        const char* ptr,
        ENFORCE_REF_PTR_) const = 0;

    /**
     * Read the value of the given symbol, and return
     * it formatted as a string; called internally by
     * DebugSymbolImpl::read
     */
    virtual SharedString* read(
        DebugSymbol*,
        DebugSymbolEvents*) const = 0;

    /**
     * Modify the symbol's value by writing the contents of
     * the given buffer to the memory location corresponding
     * to the symbol, or to the corresponding CPU registers
     */
    virtual void write(DebugSymbol*, const Buffer*) const = 0;

    virtual void attach_to_observer(Observer*) = 0;
};


/**
 * Wrapper around "naked" DataType ptrs, ensures initialization.
 */
typedef WeakPtr<DataType> WeakDataTypePtr;


#ifdef DEBUG
 #define make_pointer_name(x) \
    make_pointer_name(x, &RefPtrEnforcer(__FILE__, __LINE__))
#else
 #define make_pointer_name(x) make_pointer_name(x, NULL)
#endif
#endif // DATATYPE_H__4D27E9C4_3B4A_40B9_A302_88262FC5AA44
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
