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
#include <boost/python.hpp>
#include "zdk/get_pointer.h"
#include "zdk/properties.h"
#include "locked.h"
#include "properties.h"

using namespace boost;
using namespace boost::python;


void export_properties()
{
    class_<Properties, bases<>, noncopyable>("Properties", no_init)
        .def("get_word", &Properties::get_word, locked<>())
        .def("set_word", &Properties::set_word, locked<>())
        .def("get_double", &Properties::get_double, locked<>())
        .def("set_double", &Properties::set_double, locked<>())
        .def("get_string", &Properties::get_string, locked<>())
        .def("set_string", &Properties::set_string, locked<>())
        ;

    register_ptr_to_python<RefPtr<Properties> >();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
