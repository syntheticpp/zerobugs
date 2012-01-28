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
#include <assert.h>
#include <sstream>
#include "zdk/zobject.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/symbol.h"
#include "zdk/variant_util.h"
#include "typez/public/type_system.h"
#include "test_context.h"
#include "test_debug_symbol.h"

using namespace std;


TestContext::TestContext()
{
}


TestContext::~TestContext() throw()
{
}


bool TestContext::add_debug_symbol(RefPtr<DebugSymbol> symbol)
{
    assert(symbol.get());
    assert(symbol->name());

    string name = CHKPTR(CHKPTR(symbol->name())->c_str());
    return debugSymbolMap_.insert(make_pair(name, symbol)).second;
}


RefPtr<DataType> TestContext::lookup_type(const char* name)
{
    RefPtr<DataType> type;

    // pretend these were typedef-ed:
    if (strcmp(name, "int32_t") == 0)
    {
        RefPtr<SharedString> name = shared_string("int32_t");
        type = type_system().get_int_type(name.get(), 32, true);
    }
    else if (strcmp(name, "uint32_t") == 0)
    {
        RefPtr<SharedString> name = shared_string("uint32_t");
        type = type_system().get_int_type(name.get(), 32, false);
    }
    return type;
}


RefPtr<DebugSymbol>
TestContext::lookup_debug_symbol(const char* name, LookupOpt)
{
    assert(name);
    RefPtr<DebugSymbol> symbol;

    DebugSymbolMap::const_iterator i = debugSymbolMap_.find(name);
    if (i != debugSymbolMap_.end())
    {
        symbol = i->second;
    }
    return symbol;
}


RefPtr<DebugSymbol> TestContext::new_temp_ptr(PointerType& type, addr_t addr)
{
/*
    static RefPtr<SharedString> str = new SharedStringImpl("");

    // force exception to be thrown if it's not a pointer type
    interface_cast<PointerType&>(type);

    RefPtr<DebugSymbol> temp = new DebugSymbolImpl(
        NULL,
        *thread_,
        interface_cast<DataType&>(type),
        *str,
        addr,
        false);

    return temp;
 */
    return 0;
}


RefPtr<DebugSymbol>
TestContext::new_const(DataType& type, const string& value)
{
    return new TestDebugSymbol(type, value.c_str(), value.c_str());
}


void TestContext::commit_value(DebugSymbol& sym, Variant& var)
{
    if (DataType* type = sym.type())
    {
        ostringstream ss;
        variant_print(ss, var);

        // todo: handle VT_ARRAY and VT_OBJECT
        if (type->parse(ss.str().c_str(), 0))
        {
            sym.write(ss.str().c_str());
        }
    }
    else assert(false);
}


addr_t TestContext::push_arg(const Variant&, addr_t sp)
{
    return sp;
}


void TestContext::call_function(CallSetup&)
{
}


RefPtr<Symbol> TestContext::get_func(const CallSetup& setup) const
{
    return RefPtr<Symbol>();
}


bool TestContext::notify(DebugSymbol*)
{
    return false;
}


bool TestContext::is_expanding(DebugSymbol*) const
{
    return false;
}


static const size_t wordSize = __WORDSIZE;

TypeSystem& TestContext::type_system() const
{
    if (!types_.get())
    {
        types_.reset(new NativeTypeSystem(wordSize));
    }
    return *types_;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
