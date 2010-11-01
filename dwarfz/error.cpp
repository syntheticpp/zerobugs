//
// $Id: error.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <string>

#include "impl.h"
#include "error.h"

using namespace Dwarf;

class Error::Impl
{
public:
    explicit Impl(Dwarf_Debug dbg, Dwarf_Error err)
        : dbg_(dbg)
        , err_(err)
        , msg_(dwarf_errmsg(err_))
    {}

    Impl(const char* str, Dwarf_Debug dbg, Dwarf_Error err)
        : dbg_(dbg)
        , err_(err)
        , msg_(str)
    {
        assert(dwarf_errmsg(err_));

        msg_ += ": ";
        msg_ += dwarf_errmsg(err_);
    }

    ~Impl()
    {
        dwarf_dealloc(dbg_, err_, DW_DLA_ERROR);
    }

    const char* what() const throw()
    {
        const char* str = 0;

        try
        {
            str = msg_.c_str();
        }
        catch (...)
        {
            str = dwarf_errmsg(err_);
        }
        return str;
    }

private:
    Dwarf_Debug dbg_;
    Dwarf_Error err_;
    std::string msg_;
};


Error::Error(Dwarf_Debug dbg, Dwarf_Error err)
    : impl_(new Impl(dbg, err))
{
}


Error::Error(const char* str, Dwarf_Debug dbg, Dwarf_Error err)
    : impl_(new Impl(str, dbg, err))
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
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
