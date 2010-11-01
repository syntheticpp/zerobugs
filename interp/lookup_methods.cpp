//
// $Id: lookup_methods.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/types.h"
#include "collect_symbol.h"
#include "context.h"
#include "debug_out.h"
#include "errors.h"
#include "interp.h"
#include "lookup_methods.h"

using namespace std;


/**
 * Given a C++ class type, lookup member functions of the
 * given name. If several matches are found (overloaded methods)
 * then they are aggregated as children of the returned symbol.
 */
RefPtr<DebugSymbol> lookup_methods( Context&            ctxt,
                                    const ClassType&    klass,
                                    const string&       name,
                                    addr_t              addr)
{
    RefPtr<DebugSymbol> sym;
    bool found = false;

    const size_t numMethods = klass.method_count();
    DEBUG_OUT << klass.name() << ": " << numMethods << " method(s)\n";

    for (size_t i = 0; i != numMethods; ++i)
    {
        // fixme: const_cast
        Method* memfun = CHKPTR(const_cast<Method*>(klass.method(i)));

        if (!CHKPTR(memfun->name())->is_equal(name.c_str()))
        {
            continue;
        }
        found = true;
        RefPtr<DebugSymbol> fun;

        if (memfun->is_virtual())
        {
            if (addr)
            {
                fun = ctxt.get_virt_func(addr, memfun->vtable_offset());
                if (!fun)
                {
                    DEBUG_OUT << "get_virt_func failed\n";
                }
            }
        }
        const SharedString* linkName = memfun->linkage_name();
        if (!fun && linkName)
        {
            DEBUG_OUT << "looking up " << name
                      << " by linkage: " << linkName->c_str()
                      << endl;

            fun = ctxt.lookup_debug_symbol(linkName->c_str(),
                                           LKUP_FUNCS_ONLY);
        }
        if (fun)
        {
        #ifdef DEBUG
            if (FunType* funType = interface_cast<FunType*>(fun->type()))
            {
                DEBUG_OUT << funType->return_type()->name() << endl;
            }
            else
            {
                assert(false);
            }
            DEBUG_OUT << fun->name() << endl;
        #endif

            collect_symbol(ctxt.type_system(), sym, *fun, memfun);

            assert(sym);// post-condition

            // if enum_children is non-zero, we have overloaded methods
            // DEBUG_OUT << fun->name() << ": " << sym->enum_children() << endl;
        }
        else
        {
            DEBUG_OUT << "not found: "
                      << (linkName ? linkName->c_str() : name.c_str())
                      << endl;
        }
    }

    if (!found) // no method found, look in base classes
    {
        const size_t bcount = klass.base_count();
        for (size_t i = 0; i != bcount && !sym; ++i)
        {
            const BaseClass* base = CHKPTR(klass.base(i));

            if (ClassType* k = interface_cast<ClassType*>(base->type()))
            {
                DEBUG_OUT << "searching base class: " << k->name() << endl;
                sym = lookup_methods(ctxt, *k, name);
            }
            else
            {
                DEBUG_OUT << base->name(NULL) << ": not a class?\n";
            }
        }
    }
    else if (!sym)
    {
        // can happen if the function is declared but never defined or
        // used in the debugged program (optimized out)
        string msg = "Address of method '" + name + "' could not be found";
        throw EvalError(msg);
    }
    assert(!sym || sym->addr() || sym->enum_children());

    return sym;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
