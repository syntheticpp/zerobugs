#ifndef THREAD_DB_ERROR_H__E88D19A5_D0F4_4D05_A7C1_F32F48F5137D
#define THREAD_DB_ERROR_H__E88D19A5_D0F4_4D05_A7C1_F32F48F5137D
//
// $Id: thread_db_error.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
extern "C"
{
  #include <thread_db.h>
}
#include <string>
#include <exception>

class thread_db_error : public std::exception
{
    int err_;
    unsigned line_;
    mutable std::string what_;
    std::string file_;

public:
    thread_db_error(const char* file, unsigned line, int err)
        : err_(err)
        , line_(line)
        , file_(file)
    { }

    ~thread_db_error() throw() { }

    const char* what() const throw();
};


#define TD_ENFORCE(r) \
    while (r != TD_OK) throw thread_db_error(__FILE__, __LINE__, r);


#endif // THREAD_DB_ERROR_H__E88D19A5_D0F4_4D05_A7C1_F32F48F5137D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
