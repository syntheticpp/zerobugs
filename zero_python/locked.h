#ifndef LOCKED_H__FC3FC8C3_4BB1_48FE_B720_EFDD0D441FE0
#define LOCKED_H__FC3FC8C3_4BB1_48FE_B720_EFDD0D441FE0
//
// $Id: locked.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/python.hpp>
#include "python_mutex.h"

//
// Locked Call Policy
//
template <class Base = boost::python::default_call_policies>
struct locked : Base
{
    static PyObject* precall(PyObject*)
    {
        python_mutex().enter();
        return Py_True;
    }
    static PyObject* postcall(PyObject*, PyObject* result)
    {
        python_mutex().leave();
        return result;
    }
};
#endif // LOCKED_H__FC3FC8C3_4BB1_48FE_B720_EFDD0D441FE0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
