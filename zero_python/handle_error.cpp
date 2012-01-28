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
#include <boost/python.hpp>
#include <cStringIO.h>
#include "handle_error.h"
#include "marshaller.h"
#include "to_string.h"

using namespace std;
using namespace boost::python;


string python_get_error(handle<> exType, handle<> val, handle<> trace)
{
    string msg("Python: ");
    try
    {
        if (val)
        {
            if (const char* str = to_string(val.get()))
            {
                msg += str;
            }
        }
        else if (exType)
        {
            msg += to_string(exType.get());
        }

        PycString_IMPORT;
        if (trace && PycStringIO)
        {
            object traceOut(handle<>(PycStringIO->NewOutput(128)));
            PyTraceBack_Print(trace.get(), traceOut.ptr());
            msg += "\n";
            msg += PyString_AsString(object(
                handle<>(PycStringIO->cgetvalue(traceOut.ptr()))).ptr());
        }
    }
    catch (const exception& e)
    {
        msg += e.what() + string(__func__);
    }
    catch (...)
    {
        msg += "Unknown error in " + string(__func__);
    }
    return msg;
}



string python_get_error()
{
    PyObject* exType = 0, *value = 0, *trace = 0;
    PyErr_Fetch(&exType, &value, &trace);

    // wrap PyObjects in handles so that we don't leak
    handle<> err(allow_null(exType)),
             val(allow_null(value)),
             tr(allow_null(trace));

    return python_get_error(err, val, tr);
}


void python_handle_error()
{
    throw runtime_error(python_get_error());
}


void python_handle_error(handle<> err, handle<> val, handle<> tr)
{
    throw runtime_error(python_get_error(err, val, tr));
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
