// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: module.cpp 714 2010-10-17 10:03:52Z root $
//
#include <boost/python.hpp>
#include "zdk/get_pointer.h"
#include "zdk/module.h"
#include "zdk/shared_string.h"
#include "zdk/symbol_table.h"
#include "zdk/translation_unit.h"
#include "collector.h"
#include "locked.h"
#include "marshaller.h"
#include "module.h"

using namespace std;
using namespace boost;
using namespace boost::python;


static string module_name(const Module* module)
{
    string name;
    if (RefPtr<SharedString> moduleName = module->name())
    {
        name = moduleName->c_str();
    }
    return name;
}



typedef Collector<TranslationUnit, bool> UnitCollector;

/**
 * Enumerate translation units on main debugger thread
 */
static void
enum_units(Module* module, UnitCollector* collector)
{
    module->enum_units(collector);
}


static list translation_units(Module* module)
{
    UnitCollector callback;
    ThreadMarshaller::instance().send_command(
        bind(enum_units, module, &callback), __func__);

    return callback.get();
}


void export_module()
{
    class_<Module, bases<>, noncopyable>("Module", no_init)
        .def("name", module_name)
        .def("addr", &Module::addr)
        .def("upper", &Module::upper)
        .def("symbol_table", &Module::symbol_table_list,
            "return the symbol table for this module",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("translation_units", translation_units)
        ;
    register_ptr_to_python<RefPtr<Module> >();
}
