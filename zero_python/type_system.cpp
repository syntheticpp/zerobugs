//
// $Id: type_system.cpp 719 2010-10-22 03:59:11Z root $
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
#include "zdk/get_pointer.h"
#include "zdk/type_system.h"
#include "marshaller.h"
#include "type_system.h"

using namespace std;
using namespace boost;
using namespace boost::python;


template<typename R>
struct CallMethod0
{
    typedef void result_type;

    void operator()
     (
        RefPtr<TypeSystem> typeSys,
        R (TypeSystem::*mfun)(),
        RefPtr<DataType>& type
      )
    {
        type = (typeSys.get()->*mfun)();
    }
};


template<typename A0, typename R>
struct CallMethod1
{
    typedef void result_type;

    void operator()
     (
        RefPtr<TypeSystem> typeSys,
        R (TypeSystem::*mfun)(A0),
        A0 a0,
        RefPtr<DataType>& type
      )
    {
        type = (typeSys.get()->*mfun)(a0);
    }
};


template<typename A0, typename A1, typename A2, typename R>
struct CallMethod3
{
    typedef void result_type;

    void operator()
     (
        RefPtr<TypeSystem> typeSys,
        R (TypeSystem::*mfun)(A0, A1, A2),
        A0 a0,
        A1 a1,
        A2 a2,
        RefPtr<DataType>& type
      )
    {
        type = (typeSys.get()->*mfun)(a0, a1, a2);
    }
};



template<typename R>
static object
call_on_main_thread
(
    TypeSystem* typeSys,
    R (TypeSystem::*mfun)()
)
{
    RefPtr<DataType> type;

    ThreadMarshaller::instance().send_command(
        bind(CallMethod0<R>(), typeSys, mfun, ref(type)),
        __func__);

    return object(type);
}


template<typename A0, typename R>
static object
call_on_main_thread
(
    TypeSystem* typeSys,
    R (TypeSystem::*mfun)(A0),
    A0 a0
)
{
    RefPtr<DataType> type;

    ThreadMarshaller::instance().send_command(
        bind(CallMethod1<A0, R>(), typeSys, mfun, a0, ref(type)),
        __func__);

    return object(type);
}


template<typename A0, typename A1, typename A2, typename R>
static object
call_on_main_thread
(
    TypeSystem* typeSys,
    R (TypeSystem::*mfun)(A0, A1, A2),
    A0 a0,
    A1 a1,
    A2 a2
)
{
    RefPtr<DataType> type;

    ThreadMarshaller::instance().send_command(
        bind(CallMethod3<A0, A1, A2, R>(), typeSys, mfun, a0, a1, a2, ref(type)),
        __func__);
    return object(type);
}



static object
array_type(TypeSystem* typeSys, size_t size, RefPtr<DataType> elem)
{
    if (size) --size;

    return call_on_main_thread<int64_t, int64_t, DataType*>(
                typeSys,
                &TypeSystem::get_array_type,
                0,
                size,
                elem.get());
}


static object
class_type(TypeSystem* typeSys, string name, size_t sizeInBytes)
{
    return call_on_main_thread
             (
                typeSys,
                &TypeSystem::get_class_type,
                name.c_str(),
                sizeInBytes * sizeof(uint8_t),
                false
             );
}


static object
int_type(TypeSystem* typeSys, string name, size_t size, bool isSigned)
{
    RefPtr<SharedString> typeName =
        typeSys->get_string(name.c_str(), name.size());

    assert(typeName->ref_count() >= 2);

    return call_on_main_thread
             (
                typeSys,
                &TypeSystem::get_int_type,
                typeName.get(),
                size * sizeof(uint8_t),
                isSigned
             );
}

static object
pointer_type(TypeSystem* typeSys, RefPtr<DataType> pointedType)
{
    assert(pointedType->ref_count() >= 2);

    return call_on_main_thread
             (
                typeSys,
                &TypeSystem::get_pointer_type,
                pointedType.get()
             );
}


static object
reference_type(TypeSystem* typeSys, RefPtr<DataType> referredType)
{
    assert(referredType->ref_count() >= 2);

    return call_on_main_thread
             (
                typeSys,
                &TypeSystem::get_reference_type,
                referredType.get()
             );
}


static object string_type(TypeSystem* typeSys)
{
    return call_on_main_thread
             (
                typeSys,
                &TypeSystem::get_string_type
             );
}


static object wstring_type(TypeSystem* typeSys)
{
    return call_on_main_thread
             (
                typeSys,
                &TypeSystem::get_wide_string_type
             );
}

/**
 * Export the TypeSystem abstraction to Python, giving scripts
 * access to the debugged program's type system.
 */
void export_type_system()
{
    class_<TypeSystem, bases<>, boost::noncopyable>("TypeSystem", no_init)
        .def("array_type", array_type)
        .def("class_type", class_type)
        .def("int_type", int_type)
        .def("string_type", string_type)
        .def("wstring_type", wstring_type)
        .def("pointer_type", pointer_type)
        .def("reference_type", reference_type)
        //
        // todo: export more methods here
        //
        ;

    register_ptr_to_python<RefPtr<TypeSystem> >();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
