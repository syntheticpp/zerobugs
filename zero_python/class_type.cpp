//
// $Id: class_type.cpp 719 2010-10-22 03:59:11Z root $
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
#include "zdk/template_param.h"
#include "zdk/types.h"
#include "locked.h"


using namespace std;
using namespace boost;
using namespace boost::python;


template<typename T>
static const char* name_(const T* p)
{
    if (SharedString* name = p->name())
    {
        return name->c_str();
    }
    return "";
}


template<typename T>
static RefPtr<DataType> type_(const T* p)
{
    return p->type();
}


namespace
{
    class TemplateTypeLister : public EnumCallback<TemplateTypeParam*>
    {
        boost::python::list list_;

        void notify(TemplateTypeParam* param)
        {
            list_.append(RefPtr<TemplateTypeParam>(param));
        }

    public:
        boost::python::list list() const { return list_; }
    };
}


static list template_types_(const ClassType* klass)
{
    TemplateTypeLister helper;
    klass->enum_template_type_param(&helper);

    return helper.list();
}


void export_class_type()
{
    class_<TemplateTypeParam, bases<>, noncopyable>("TemplateType", no_init)
        .def("name", name_<TemplateTypeParam>)
        .def("type", type_<TemplateTypeParam>)
        ;

    class_<BaseClass, bases<>, noncopyable>("ClassBase", no_init)
        .def("type", type_<BaseClass>)
        ;

    class_<Member, bases<>, noncopyable>("ClassMember", no_init)
        .def("name", name_<Member>)
        .def("type", type_<Member>)
        .def("is_static", &Member::is_static)
        ;

    class_<Method, bases<>, noncopyable>("ClassMethod", no_init)
        .def("access", &Method::access)
        .def("name", name_<Method>)
        .def("type", type_<Method>)
        .def("is_inline", &Method::is_inline)
        .def("is_static", &Method::is_static)
        .def("is_virtual", &Method::is_virtual)
        .def("start_addr", &Method::start_addr)
        .def("end_addr", &Method::end_addr)
        ;

    class_<ClassType, bases<DataType>, noncopyable>("ClassType", no_init)
        .def("base", &ClassType::base,
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("base_count", &ClassType::base_count)
        .def("is_union", &ClassType::is_union)
        .def("member", &ClassType::member,
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("method", &ClassType::method,
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("member_count", &ClassType::member_count)
        .def("method_count", &ClassType::method_count)
        .def("template_types", template_types_)
        .def("virtual_base_count", &ClassType::virtual_base_count)
        ;

    enum_<Access>("Access")
        .value("private", ACCESS_PRIVATE)
        .value("protected", ACCESS_PROTECTED)
        .value("public", ACCESS_PUBLIC)
        ;

    register_ptr_to_python<RefPtr<ClassType> >();
    register_ptr_to_python<RefPtr<TemplateTypeParam> >();
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
