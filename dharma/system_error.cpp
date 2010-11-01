//
// $Id: system_error.cpp 710 2010-10-16 07:09:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <errno.h>      // errno
#include <iostream>
#include <string.h>     // strerror
#include "environ.h"
#include "system_error.h"


#if DEBUG
////////////////////////////////////////////////////////////////
//
static bool abort_on_error()
{
    static const bool flag = env::get_bool("ZERO_ABORT_ON_ERROR");
    return flag;
}

#define CHECK_ABORT_FLAG() while (abort_on_error()) abort()
#else
 #define CHECK_ABORT_FLAG()
#endif


////////////////////////////////////////////////////////////////
SystemError::~SystemError() throw()
{
}


////////////////////////////////////////////////////////////////
SystemError::SystemError(int errcode)
    : error_(errcode ? errcode : errno)
    , formatted_(false)
{
    CHECK_ABORT_FLAG();
}


////////////////////////////////////////////////////////////////
SystemError::SystemError(int errcode, const std::string& what, bool debug)
    : error_(errcode)
    , formatted_(true)
    , what_(what)
{
    if (debug)
    {
        CHECK_ABORT_FLAG();
    }
}


////////////////////////////////////////////////////////////////
SystemError::SystemError(const std::string& msg, int errcode, bool debug)
try
  : error_(errcode ? errcode : errno)
  , formatted_(false)
  , what_(msg)
{
    if (debug)
    {
        CHECK_ABORT_FLAG();
    }
}
catch (...)
{
}


////////////////////////////////////////////////////////////////
const char* SystemError::what() const throw()
{
    if (!formatted_)
    {
        try
        {
            if (!what_.empty())
            {
                what_ += ": ";
            }
            what_ += ::strerror(error_);

            formatted_ = true;
        }
        catch (...)
        {
        }
    }
    return what_.c_str();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
