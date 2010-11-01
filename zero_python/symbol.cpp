//
// $Id: symbol.cpp 719 2010-10-22 03:59:11Z root $
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
#include "generic/lock.h"
#include "zdk/get_pointer.h"
#include "zdk/mutex.h"
#include "zdk/symbol.h"
#include "zdk/symbol_table.h"
#include "locked.h"
#include "symbol.h"

using namespace std;
using namespace boost::python;


static string symbol_name(const Symbol* sym)
{
    if (RefPtr<SharedString> name = sym->name())
    {
        //return Py_DecodeUTF8(name->c_str(), name->length(), "ignore");
        return name->c_str();
    }
    return string();
}


enum DemangleOptions
{
    DEMANGLE_NAME_ONLY,
    DEMANGLE_PARAM,
};


static string
symbol_demangle(const Symbol* sym, DemangleOptions opt = DEMANGLE_NAME_ONLY)
{
    Lock<Mutex> lock(python_mutex());

    if (RefPtr<SharedString> name = sym->demangled_name(opt))
    {
        return name->c_str();
    }
    return string();
}

BOOST_PYTHON_FUNCTION_OVERLOADS(demangle_overloads, symbol_demangle, 1, 2)


static const char* symbol_filename(const Symbol* sym)
{
    const char* filename = "";
    if (RefPtr<SharedString> file = sym->file())
    {
        filename = file->c_str();
    }
    return filename;
}


void export_symbol()
{
    register_ptr_to_python<RefPtr<Symbol> >();

scope in_Symbol =
    class_<Symbol, bases<>, boost::noncopyable>("Symbol", no_init)
        .def("addr", &Symbol::addr, "symbol's address", locked<>()
            )
        .def("filename", &symbol_filename,
            "source file name where symbol is defined", locked<>()
            )
        .def("line", &Symbol::line,
            "source line where symbol is defined", locked<>()
            )
        .def("offset", &Symbol::offset,
            "symbol's offset from nearest function"
            )
        .def("name", symbol_name, "symbol name (possibly mangled)",
            locked<>()
            )
        .def("demangle", symbol_demangle,
            demangle_overloads(args("options"), "return demangled symbol name")
            )
        .def("table", &Symbol::table,
             "return the table that owns this symbol",
             return_value_policy<reference_existing_object>()
            )
        .def("value", &Symbol::value, "value in symbol table", locked<>())
        ;

    enum_<DemangleOptions>("Demangle")
        .value("NameOnly", DEMANGLE_NAME_ONLY)
        .value("Param", DEMANGLE_PARAM)
        ;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
