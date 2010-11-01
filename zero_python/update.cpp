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
// $Id: update.cpp 714 2010-10-17 10:03:52Z root $
//
#include <boost/python.hpp>
#include <zdk/get_pointer.h>
#include "zero_python/update.h"

using namespace std;
using namespace boost;
using namespace boost::python;


void export_update()
{
    class_<UpdateImpl, noncopyable>("Update", init<string, string>())
        .def("url", &UpdateImpl::url)
        .def("description", &UpdateImpl::description)
        .def("apply", &UpdateImpl::apply)
        ;

    register_ptr_to_python<RefPtr<UpdateImpl> >();
}

