//
// $Id: error.cpp 711 2010-10-16 07:09:23Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <string>
#include <libelf.h>

#include "public/error.h"


class ELF::Error::Impl
{
public:
    Impl(const std::string& msg, int err)
        : err_(err)
        , msg_(msg)
    {
        if (const char* errmsg = elf_errmsg(err_))
        {
            msg_ += ": ELF ";
            msg_ += errmsg;
        }
        else
        {
            assert(err_ == 0);
        }
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
            str = elf_errmsg(err_); // fallback
        }

        return str;
    }

private:
    int err_;
    std::string msg_;
};


ELF::Error::Error(const char* msg)
{
    assert(msg);

    impl_.reset(new Impl(msg, elf_errno()));
}


ELF::Error::~Error() throw()
{
}


const char* ELF::Error::what() const throw()
{
    assert(impl_);

    return impl_->what();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
