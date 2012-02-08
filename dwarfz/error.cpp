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

#include "zdk/log.h"
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>

#include "impl.h"
#include "error.h"

using namespace Dwarf;


class Error::Impl
{
public:
    Impl(const char* str, Dwarf_Debug dbg, Dwarf_Error err)
        : dbg_(dbg)
        , err_(err)
        , msg_(str)
    {
        assert(dwarf_errmsg(err_));
        if (!msg_.empty())
        {
            msg_ += ": ";
        }
        msg_ += dwarf_errmsg(err_);
    }

    ~Impl()
    {
        dwarf_dealloc(dbg_, err_, DW_DLA_ERROR);
    }

    const char* what() const throw()
    {
        return msg_.c_str();
    }

    std::ostream&
    log(std::ostream& out, const char* file, size_t line) const
    {
        return out << file << ':' << line << ' ' << what();
    } 

private:
    Dwarf_Debug dbg_;
    Dwarf_Error err_;
    std::string msg_;
};



Error::Error(const char* str, Dwarf_Debug dbg, Dwarf_Error err)
    : impl_(std::make_shared<Impl>(str, dbg, err))
{
}


Error::~Error() throw()
{
}


const char* Error::what() const throw()
{
    assert(impl_);
    return impl_->what();
}


void Error::Throw(
    const char* func,
    Dwarf_Debug dbg,
    Dwarf_Error err,
    const char* file,
    size_t      line)

{
    Error e(func, dbg, err);

    if (file)
    {
        e.impl_->log(
            Log::Level(__FILE__, __LINE__, Log::ALWAYS), file, line)
            << std::endl;
    }

    throw e;
}


std::string Error::Message(
    Dwarf_Debug dbg,
    Dwarf_Error err,
    const char* file,
    size_t      line)

{
    Error e("", dbg, err);
    std::string result(e.what());

    
    if (file)
    {
        std::ostringstream ss;

        e.impl_->log(ss, file, line);

        result = ss.str() + ": " + result;
    }

    return result;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
