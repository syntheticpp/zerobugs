#ifndef CALL_H__285C101D_82AD_4FAA_BF92_7DF99A28413C
#define CALL_H__285C101D_82AD_4FAA_BF92_7DF99A28413C
//
// $Id$
//
// Templates for calling into Python code
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include "zero_python/handle_error.h"
#include "zero_python/python_embed.h"

#define CATCH_AND_HANDLE(f) \
    catch (const std::exception& e) { \
        std::cerr << f << ": " << e.what() << std::endl; \
    } catch (...) { python_handle_error(); }

#if PY_MAJOR_VERSION < 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION < 3)
//
// Python 2.2 API is not const-correct
//
 #define CONST // as nothing
#else
 #define CONST const
#endif

PythonEmbed& py_interp();


//
// Call script function with no parameters
//
template<typename R>
static inline R call_(CONST char* fname)
{
    try
    {
        if (PyObject* callback =
            PyDict_GetItemString(py_interp().main_ns().ptr(), fname))
        {
            return boost::python::call<R>(callback);
        }
    }
    CATCH_AND_HANDLE(fname)
    return R();
}


//
// Call script function with one param
//
template<typename R, typename A>
static inline R call_(CONST char* fname, A arg)
{
    try
    {
        if (PyObject* callback =
            PyDict_GetItemString(py_interp().main_ns().ptr(), fname))
        {
            return boost::python::call<R>(callback, arg);
        }
    }
    CATCH_AND_HANDLE(fname)
    return R();
}


//
// Call function with 2 params
//
template<typename R, typename A1, typename A2>
static inline R call_(CONST char* fname, A1 arg1, A2 arg2)
{
    try
    {
        if (PyObject* callback =
            PyDict_GetItemString(py_interp().main_ns().ptr(), fname))
        {
            return boost::python::call<R>(callback, arg1, arg2);
        }
    }
    CATCH_AND_HANDLE(fname)
    return R();
}


template<typename R, typename A1, typename A2, typename A3>
static inline R call_(CONST char* fname, A1 arg1, A2 arg2, A3 arg3)
{
    try
    {
        if (PyObject* callback =
            PyDict_GetItemString(py_interp().main_ns().ptr(), fname))
        {
            return boost::python::call<R>(callback, arg1, arg2, arg3);
        }
    }
    CATCH_AND_HANDLE(fname)
    return R();
}
#endif // CALL_H__285C101D_82AD_4FAA_BF92_7DF99A28413C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
