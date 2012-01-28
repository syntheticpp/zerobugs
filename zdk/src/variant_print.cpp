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

#include <iomanip>
#include <iostream>
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/types.h"
#include "zdk/variant_util.h"
#include "generic/state_saver.h"

using namespace std;


static void
prepare_float_output(ostream& out, size_t prec)
{
    //out.setf(ios::fixed, ios::floatfield);
    //out.setf(ios::showpoint);
    //out.precision(prec);
}


static bool print_value(ostream& out, const Variant& v)
{
    if (DebugSymbol* sym = v.debug_symbol())
    {
        if (SharedString* value = sym->value())
        {
            out << value;
            return true;
        }
    }
    return false;
}


ostream&
variant_print(ostream& os, const Variant& v, int base)
{
    StateSaver<ios, ios::fmtflags> state(os);
    if (base)
    {
        os << showbase;
        switch(base)
        {
        case 10: os << dec; break;
        case 16: os << hex; break;
        case  8: os << oct; break;
        default: throw invalid_argument("invalid base");
        }
    }

    switch (v.type_tag())
    {
    case Variant::VT_BOOL:
        os << static_cast<bool>(v.bits());
        break;

    case Variant::VT_INT8:
    case Variant::VT_INT16:
    case Variant::VT_INT32:
        os << static_cast<int>(v.int64());
        break;

    case Variant::VT_UINT8:
    case Variant::VT_UINT16:
    case Variant::VT_UINT32:
        os << static_cast<unsigned int>(v.int64());
        break;

    case Variant::VT_INT64:
        os << v.int64();
        break;

    case Variant::VT_UINT64:
        os << v.uint64();
        break;

    case Variant::VT_FLOAT:
        prepare_float_output(os, numeric_limits<float>::digits10);
        os << v.long_double();
        break;
    case Variant::VT_DOUBLE:
        prepare_float_output(os, numeric_limits<double>::digits10);
        os << v.long_double();
        break;
    case Variant::VT_LONG_DOUBLE:
        prepare_float_output(os, numeric_limits<long double>::digits10);
        os << v.long_double();
        break;

    case Variant::VT_POINTER:

        if (v.encoding() == Variant::VE_STRING)
        {
            if (DebugSymbol* sym = v.debug_symbol())
            {
                os << sym->value();
                break;
            }
        }

        if (base == 0)
        {
            os << hex;
        }
        os << showbase << v.pointer();
        break;

    case Variant::VT_ARRAY:
        if (!print_value(os, v))
        {
            os << "[array]";
        }
        break;

    case Variant::VT_OBJECT:
        if (!print_value(os, v))
        {
            os << "{object}";
        }
        break;

    case Variant::VT_NONE: os << "<none>"; break;
    case Variant::VT_VOID: os << "<void>"; break;
    }
    return os;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
