// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: zobject.cpp 714 2010-10-17 10:03:52Z root $
//
#include <boost/python.hpp>
#include "zdk/get_pointer.h"
#include "zdk/zobject.h"

using namespace boost::python;


void export_zobject()
{
    class_<ZObject, bases<>, boost::noncopyable>("ZObject", no_init)
        ;

    register_ptr_to_python<RefPtr<ZObject> >();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
