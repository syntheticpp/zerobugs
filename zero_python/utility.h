#ifndef UTILITY_H__C7F65688_5AB3_4860_852D_512453295CB8
#define UTILITY_H__C7F65688_5AB3_4860_852D_512453295CB8
//
// $Id: utility.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <boost/utility.hpp>
#include "generic/auto.h"
#include "zdk/properties.h"
#include "handle_error.h"
#include "to_string.h"


class MainThreadScope : Automatic
{
public:
    MainThreadScope();
    ~MainThreadScope();
};


void
python_dict_to_properties(boost::python::dict, Properties&);


inline size_t __len__(boost::python::object obj)
{
    return boost::python::extract<size_t>(obj.attr("__len__")());
}


#endif // UTILITY_H__C7F65688_5AB3_4860_852D_512453295CB8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
