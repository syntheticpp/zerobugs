//
// $Id: eval_inc_dec.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sstream>
#include <stdexcept>
#include "zdk/types.h"
#include "context.h"
#include "eval_inc_dec.h"
#include "errors.h"
#include "variant_assign.h"
#include "variant_impl.h"

using namespace std;


/**
 * Evaluate increment or decrement operator
 */
RefPtr<Variant> eval_inc_dec(
    Context&        context,
    const DataType& type,
    const Variant&  lval,
    bool            increment)
{
    RefPtr<Variant> result = new VariantImpl;
    DebugSymbol* sym = lval.debug_symbol();

    if (sym && (sym->is_constant() || sym->is_return_value()))
    {
        throw EvalError(std::string("non-lvalue in ")
                        + (increment ? "increment" : "decrement"));
    }

    if (lval.type_tag() == Variant::VT_POINTER)
    {
        const PointerType& ptrType = interface_cast<const PointerType&>(type);

        if (!ptrType.pointed_type() || !ptrType.pointed_type()->size())
        {
            throw EvalError("can't increment/decrement pointer of type void*");
        }
        const size_t size = ptrType.pointed_type()->size();
        addr_t addr = lval.pointer() + (increment ? size : -size);

        variant_assign(*result, type, addr);
    }
    else if (is_integer(lval))
    {
        if (lval.type_tag() == Variant::VT_UINT64)
        {
            uint64_t v = lval.uint64() + (increment ? 1 : -1);
            variant_assign(*result, type, v);
        }
        else
        {
            int64_t v = lval.int64() + (increment ? 1 : -1);
            variant_assign(*result, type, v);
        }
    }
    else if (is_float(lval))
    {
        long double v = lval.long_double() + (increment ? 1 : -1);
        variant_assign(*result, type, v);
    }
    else
    {
        ostringstream err;

        err << "type ";
        if (DebugSymbol* sym = lval.debug_symbol())
        {
            err << sym->type_name() << " ";
        }
        err << "does not support "
            << (increment ? "increment" : "decrement")
            << " operator";

        throw runtime_error(err.str());
    }

    // commit the incremented/decremented value to memory
    if (sym)
    {
        assert(result->type_tag() != Variant::VT_NONE);
        assert(result->size());

        context.commit_value(*sym, *result);

        result = new VariantImpl(*sym);
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
