#ifndef SYSTEM_ERROR_H__1056608775
#define SYSTEM_ERROR_H__1056608775
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
#include "zdk/export.h"
#include "zdk/stdexcept.h"
#include <string>

/**
 * An exception class that wrapps
 * error codes returned by system calls.
 */
class ZDK_EXPORT SystemError : public std::exception
{
public:
    explicit SystemError(int error = 0);
    explicit SystemError(const std::string& prefix,
                         int error = 0,
                         bool debug = true);
    SystemError(int error, const std::string& what, bool debug = true);

    ~SystemError() throw();
    const char* what() const throw();

    int error() const { return error_; }

private:
    const int error_;
    mutable bool formatted_;
    mutable std::string what_;
};
#endif // SYSTEM_ERROR_H__1056608775
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
