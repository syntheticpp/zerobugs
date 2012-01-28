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
#include "zdk/check_ptr.h"
#include "zdk/types.h"
#include "zdk/get_pointer.h"
#include "data_type.h"

using namespace std;
using namespace boost::python;


static string type_name(const DataType* type)
{
    RefPtr<SharedString> v;
    if (type)
    {
        v = type->name();
    }
    return v ? CHKPTR(v->c_str()) : "";
}


static bool is_ptr(const DataType* type)
{
    return interface_cast<const PointerType*>(type);
}


static bool is_ref(const DataType* type)
{
    if (const PointerType* p = interface_cast<const PointerType*>(type))
    {
        return p->is_reference();
    }
    return false;
}


static object pointed_type(const DataType* type)
{
    RefPtr<DataType> result;

    if (const PointerType* ptr = interface_cast<const PointerType*>(type))
    {
        result = ptr->pointed_type();
    }
    return object(result);
}


static object as_class(DataType* type)
{
    RefPtr<ClassType> result;

    if (ClassType* klass = interface_cast<ClassType*>(type))
    {
        result = klass;
    }
    return object(result);
}


void export_data_type()
{
    class_<DataType, bases<>, boost::noncopyable>("DataType", no_init)
        .def("as_class", as_class)
        .def("bit_size", &DataType::bit_size)
        .def("is_fundamental", &DataType::is_fundamental)
        .def("is_pointer", is_ptr)
        .def("is_reference", is_ref)
        .def("name", type_name)
        .def("pointed_type", pointed_type)
        .def("size", &DataType::size)
        ;
    ;

    register_ptr_to_python<RefPtr<DataType> >();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
