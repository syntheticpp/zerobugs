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

#include "zdk/data_type.h"
#include "debug_out.h"
#include "errors.h"
#include "eval_sym.h"
#include "interp.h"
#include "variant_impl.h"

using namespace std;


RefPtr<Variant>
eval_sym(Context& context, const string& name, RefPtr<DebugSymbol> sym)
{
    if (!sym)
    {
        throw EvalError("debug symbol not found: " + name);
    }
    if (!sym->value())
    {
        sym->read(&context);
    }
    else
    {
        DEBUG_OUT << name << "=" << sym->value() << endl;
    }
    RefPtr<DataType> type = sym->type();

    // expect the type to be known at eval-time
    if (!type)
    {
        throw logic_error(name + ": unknown type");
    }
    DEBUG_OUT << type->name() << endl;
    return new VariantImpl(*sym);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
