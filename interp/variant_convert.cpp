//
// $Id: variant_convert.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <stdexcept>
#include <sstream>
#include "zdk/data_type.h"
#include "zdk/variant_util.h"
#include "zdk/types.h"
#include "variant_convert.h"
#include "variant_impl.h"
#include "debug_out.h"
#ifdef DEBUG
 #include "interp.h"
#endif


using namespace std;


RefPtr<Variant> variant_convert(RefPtr<Variant>& var, DataType& type)
{
    assert(var.get());
    RefPtr<VariantImpl> result;

    if (DebugSymbol* dsym = var->debug_symbol())
    {
        RefPtr<DataType> other = dsym->type();

        if (type.is_equal(other.get()))
        {
        #ifdef DEBUG
            clog << __func__ << ": same type (" << type.name() << ")\n";
        #endif
            return var;
        }

        //interface_cast<DebugSymbolImpl*>(dsym)->remove_all_children();

        RefPtr<DebugSymbol> clone;

        if (other && other->size() != type.size())
        {
            // If types are of different sizes, then
            // create a constant clone; otherwise we might
            // attempt later to refresh (read the object)
            // which is incorrect; the clone does not
            // correspond to an actual variable in the
            // debugged program.

            clone = dsym->clone(dsym->value(), &type, true);
        }
        else
        {
            clone = dsym->clone(NULL, &type);
        }

        assert(clone);
        assert(clone->value());

        DEBUG_OUT << clone->value() << ", " << clone->type()->name() << endl;

        result = new VariantImpl(*clone);
    }
    else
    {
        ostringstream tmp;
        variant_print(tmp, *var);
        assert(var->type_tag() != Variant::VT_OBJECT);
        DEBUG_OUT << type.name() << endl;
        result = new VariantImpl;

        if (!type.parse(tmp.str().c_str(), result.get()))
        {
            cerr << "*** Warning: " << __func__ << ": ";
            cerr << type.name() << ".parse failed on: ";
            cerr << tmp.str() << endl;
        }
    }
    assert(result.get());
    return var = result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
