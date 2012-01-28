#ifndef TO_STRING_H__2A39F103_A0CD_4CD5_B524_570072B87BF7
#define TO_STRING_H__2A39F103_A0CD_4CD5_B524_570072B87BF7
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

#include "zdk/export.h"
#include <boost/python.hpp>


inline ZDK_LOCAL const char* to_string(PyObject* obj)
{
    if (obj) try
    {
        boost::python::handle<> h(PyObject_Str(obj));

        if (const char* str = PyString_AsString(h.get()))
        {
            return str;
        }
    }
    catch (...)
    {
    }
    return "";
}


inline ZDK_LOCAL const char* to_string(const boost::python::object& obj)
{
    return to_string(obj.ptr());
}


inline ZDK_LOCAL const char* to_string(const boost::python::handle<>& h)
{
    return to_string(h.get());
}


/*
inline ZDK_LOCAL const char* __str__(const boost::python::object& obj)
{
    return to_string(obj.ptr());
}


inline ZDK_LOCAL const char* __repr__(PyObject* obj)
{
    if (obj) try
    {
        boost::python::handle<> h(PyObject_Repr(obj));
        if (const char* str = PyString_AsString(boost::python::object(h).ptr()))
            return str;
    }
    catch (...)
    {
    }
    return "";
}


inline ZDK_LOCAL const char* __repr__(const boost::python::object& obj)
{
    return repr(obj.ptr());
}
*/

#endif // TO_STRING_H__2A39F103_A0CD_4CD5_B524_570072B87BF7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
