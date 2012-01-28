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
#include <boost/python.hpp>
#include "zdk/data_type.h"
#include "zdk/get_pointer.h"
#include "debug_symbol.h"
#include "debug_sym_wrap.h"
#include "locked.h"

using namespace boost::python;


void export_debug_symbol()
{
    class_<DebugSymbolWrap, boost::noncopyable>("DebugSymbol", no_init)
        .def("add_child", &DebugSymbolWrap::add_child)
        .def("addr", &DebugSymbolWrap::addr)
        .def("children", &DebugSymbolWrap::children)
        .def("create", &DebugSymbolWrap::create)
        .def("name", &DebugSymbolWrap::name)
        .def("type", &DebugSymbolWrap::type)
        .def("value", &DebugSymbolWrap::value)
        .def("type_name", &DebugSymbolWrap::type_name)
        .def("set_type_name", &DebugSymbolWrap::set_type_name)
        .def("set_constant", &DebugSymbolWrap::set_constant, locked<>())
        .def("set_value", &DebugSymbolWrap::set_value, locked<>())
        .def("has_children", &DebugSymbolWrap::has_children)
        .def("is_const", &DebugSymbolWrap::is_constant)
        .def("is_expanding", &DebugSymbolWrap::is_expanding)
        .def("is_expanded", &DebugSymbolWrap::is_expanded)
        .def("set_expanded", &DebugSymbolWrap::set_expanded)
        .def("thread", &DebugSymbolWrap::thread,
             "return the process that owns this symbol"
            )
        .def("typesys", &DebugSymbolWrap::type_system)
        .def("type_system", &DebugSymbolWrap::type_system)
        .def("process", &DebugSymbolWrap::process,
             "return the process that owns this symbol"
            )
        .def("read", &DebugSymbolWrap::read)
        .def("set_numeric_base", &DebugSymbolWrap::set_numeric_base)
        .def("set_tooltip", &DebugSymbolWrap::set_tooltip)
        .def("tooltip", &DebugSymbolWrap::tooltip)
    ;

    register_ptr_to_python<RefPtr<DebugSymbolWrap> >();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
