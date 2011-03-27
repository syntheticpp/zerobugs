#ifndef VARIANT_H__47EB6E22_F451_499E_B32B_409B77A7683D
#define VARIANT_H__47EB6E22_F451_499E_B32B_409B77A7683D
//
// $Id: variant.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/platform.h"
#include "zdk/zobject.h"

struct DebugSymbol;


DECLARE_ZDK_INTERFACE_(Variant, ZObject)
{
    DECLARE_UUID("32199c51-2f32-4e87-b4f6-34fc7a914d78")

    enum TypeTag
    {
        VT_NONE = -1,
        VT_VOID,
        VT_INT8,
        VT_UINT8,
        VT_INT16,
        VT_UINT16,
        VT_INT32,
        VT_UINT32,
        VT_INT64,
        VT_UINT64,

        VT_FLOAT,
        VT_DOUBLE,
        VT_LONG_DOUBLE,
        VT_POINTER,

        VT_ARRAY,
        VT_OBJECT,

        VT_BOOL,

        VT_INT       = VT_INT32,
        VT_UINT      = VT_UINT32,
        VT_LONGLONG  = VT_INT64,
        VT_ULONGLONG = VT_UINT64,

#if (__WORDSIZE == 32)

        VT_LONG      = VT_INT32,
        VT_ULONG     = VT_UINT32,

#elif (__WORDSIZE == 64)

        VT_LONG      = VT_INT64,
        VT_ULONG     = VT_UINT64,
#endif
    };

    enum Encoding
    {
        VE_BINARY,
        VE_STRING,
    };

    virtual void set_type_tag(TypeTag) = 0;

    virtual TypeTag type_tag() const = 0;

    virtual size_t size() const = 0;

    virtual uint64_t uint64() const = 0;

    virtual int64_t int64() const = 0;

    virtual long double long_double() const = 0;

    virtual Platform::addr_t pointer() const = 0;

    virtual uint64_t bits() const = 0;

    virtual DebugSymbol* debug_symbol() const = 0;

    virtual const void* data() const = 0;

    virtual void copy(const Variant*, bool lvalue) = 0;

    virtual Encoding encoding() const = 0;
};


bool inline is_integer(const Variant& v)
{
    switch (v.type_tag())
    {
    default:
        break;

    case Variant::VT_INT8:
    case Variant::VT_INT16:
    case Variant::VT_INT32:
    case Variant::VT_INT64:
    case Variant::VT_UINT8:
    case Variant::VT_UINT16:
    case Variant::VT_UINT32:
    case Variant::VT_UINT64:
        return true;
    }
    return false;
}


bool inline is_float(const Variant& v)
{
    switch (v.type_tag())
    {
    default:
        break;

    case Variant::VT_FLOAT:
    case Variant::VT_DOUBLE:
    case Variant::VT_LONG_DOUBLE:
        return true;
    }
    return false;
}


bool inline is_signed_int(const Variant& v)
{
    switch (v.type_tag())
    {
    default: break;
    case Variant::VT_INT8:
    case Variant::VT_INT16:
    case Variant::VT_INT32:
    case Variant::VT_INT64:
        return true;
    }
    return false;
}

#endif // VARIANT_H__47EB6E22_F451_499E_B32B_409B77A7683D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
