#ifndef HANDLE_ERROR_H__070083E0_1293_45F4_B324_202DFD496320
#define HANDLE_ERROR_H__070083E0_1293_45F4_B324_202DFD496320
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

#include <string>
#include <boost/python.hpp>

/**
 * Handle error on Python thread
 */
void python_handle_error();

void python_handle_error(boost::python::handle<>,
                         boost::python::handle<>,
                         boost::python::handle<>);

std::string python_get_error();

std::string python_get_error(boost::python::handle<> exType,
                             boost::python::handle<> val,
                             boost::python::handle<> trace);

#endif // HANDLE_ERROR_H__070083E0_1293_45F4_B324_202DFD496320
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
