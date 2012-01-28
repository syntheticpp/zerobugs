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
//
#include <assert.h>
#include "utility.h"
#include "marshaller.h"

using namespace std;
using namespace boost::python;


MainThreadScope::MainThreadScope()
{
}


MainThreadScope::~MainThreadScope()
{
    ThreadMarshaller::instance().unblock_waiting_threads();
}


static void add_object(Properties& prop, const char* name, object obj)
{
    object type = obj.attr("__class__");
    string typeName = to_string(type.attr("__name__"));

    if (typeName == "int")
    {
        word_t val = extract<word_t>(obj);
        prop.set_word(name, val);
    }
    else if (typeName == "float")
    {
        double val = extract<double>(obj);
        prop.set_double(name, val);
    }
  /*
    else if (typeName == "bool")
    {
        bool val = extract<bool>(obj);
        prop.set_word(name, val);
    }
   */
    else if (typeName == "str")
    {
        const char* str = extract<const char*>(obj);
        prop.set_string(name, str);
    }
    else
    {
        throw runtime_error("parameter type not supported: " + typeName);
    }
}


void
python_dict_to_properties(dict d, Properties& prop)
{
    const size_t len = __len__(d);

    object iter = d.iterkeys();

    for (size_t i = 0; i != len; ++i)
    {
        object key = iter.attr("next")();

        add_object(prop, to_string(key), d[key]);
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
