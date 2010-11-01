//
// $Id: filter.cpp 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/python.hpp>
#include "dharma/variant_impl.h"
#include "typez/public/debug_symbol.h"
#include "zdk/debug_sym.h"
#include "zdk/properties.h"
#include "zdk/type_tags.h"
#include "zero_python/call.h"
#include "zero_python/debug_sym_wrap.h"
#include "zero_python/to_string.h"
#include "zero_python/utility.h"
#include "interp.h"
#include "filter.h"


using namespace std;
using namespace boost;
using namespace boost::python;


UserDataFilter::UserDataFilter(WeakPtr<Python>& interp)
{
    interp_ = interp.lock();

    if (!interp_)
    {
        interp_.reset(new Python);
        interp = interp_;
    }
}


UserDataFilter::~UserDataFilter() throw()
{
}



DebugSymbol* UserDataFilter::transform
(
    DebugSymbol* symbol,
    DebugSymbol* parent,
    DebugSymbolEvents* events
) const
{
    assert(symbol);

    if (symbol) try
    {
        RefPtr<DebugSymbolWrap> wrap(new DebugSymbolWrap(symbol, events));
        //
        // call user-defined on_debug_symbol method, implemented as Python script
        //
        object ret = call_<object>("on_debug_symbol", wrap);
        RefPtr<DebugSymbolWrap> newsym;

        if (ret)
        {
            newsym = extract<RefPtr<DebugSymbolWrap> >(ret);
        }
        if (newsym)
        {
            if (DebugSymbol* retsym = newsym->detach())
            {
                return retsym;
            }
        }
    }
    catch (const error_already_set&)
    {
        cerr <<  python_get_error() << endl;
    }
    catch (const std::exception& e)
    {
        cerr << e.what() << endl;
    }
    return NULL;
}



bool UserDataFilter::hide(const DebugSymbol* symbol) const
{
    return false;
}


static RefPtr<Variant> object_to_var(object obj)
{
    RefPtr<VariantImpl> var(new VariantImpl);

    object type = obj.attr("__class__");
    string name = to_string(type.attr("__name__"));

    if (name == "int")
    {
        int val = extract<int>(obj);
        put(var.get(), val);
    }
    else if (name == "float")
    {
        double val = extract<double>(obj);
        put(var.get(), val);
    }
    else if (name == "bool")
    {
        bool val = extract<bool>(obj);
        put(var.get(), val);
    }
    else
    {
        throw runtime_error("parameter type not supported: " + name);
    }
    return var;
}


size_t
UserDataFilter::enum_params(EnumCallback3<const char*,
                                          const char*,
                                          const Variant*,
                                          bool>* callback) const
{
    try
    {
        dict d = call_<dict>("get_configurable_params");

        const size_t len = __len__(d);
        if (!callback)
        {
            return len;
        }

        object iter = d.iterkeys();

        for (size_t i = 0; i != len; ++i)
        {
            object key = iter.attr("next")();
            object val = d[key];

            if (RefPtr<Variant> var = object_to_var(val[0]))
            {
                if (!callback->notify(to_string(key), to_string(val[1]), var.get()))
                {
                    return i;
                }
            }
        }
        return len;
    }
    catch (const error_already_set&)
    {
        throw runtime_error(python_get_error());
    }
    return 0;
}



/**
 * @note not necessarily on the main thread; problem?
 */
void
UserDataFilter::on_param_change(Properties* prop, const char* name)
{
    call_<void>("on_param_change", ref(*prop), name);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
