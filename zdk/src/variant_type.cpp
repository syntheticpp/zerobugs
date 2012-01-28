// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "zdk/variant_util.h"

static const char* tag_name[] =
{
    "none",                 // VT_NONE = -1,
    "void",                 // VT_VOID,
    "int8_t",               // VT_INT8,
    "uint8_t",              // VT_UINT8,
    "int16_t",              // VT_INT16,
    "uint16_t",             // VT_UINT16,
    "int32_t",              // VT_INT32,
    "uint32_t",             // VT_UINT32,
    "int64_t",              // VT_INT64,
    "uint64_t",             // VT_UINT64,
    "float",                // VT_FLOAT,
    "double",               // VT_DOUBLE,
    "long double",          // VT_LONG_DOUBLE,
    "pointer",              // VT_POINTER,
    "array",                // VT_ARRAY,
    "object",               // VT_OBJECT,
};

const char* variant_type(Variant::TypeTag tag)
{
    if (tag - Variant::VT_NONE >= int(sizeof(tag_name)/sizeof(tag_name[0])))
    {
        return "unknown type tag";
    }
    return tag_name[tag - Variant::VT_NONE];
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
