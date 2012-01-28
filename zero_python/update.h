#ifndef UPDATE_H__A99CB672_4EE5_42EA_A097_AF831CA33FB6
#define UPDATE_H__A99CB672_4EE5_42EA_A097_AF831CA33FB6
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include <boost/python.hpp>
#include <string>
#include <zdk/update.h>
#include <zdk/zobject_impl.h>


CLASS UpdateImpl
    : public ZObjectImpl<Update>
    , public boost::python::wrapper<Update>
{
    std::string url_;
    std::string description_;

    UpdateImpl(const UpdateImpl& other)
        : boost::python::wrapper<Update>(other)
        , url_(other.url_)
        , description_(other.description_)
    { }

public:
    UpdateImpl(const std::string& url, const std::string& desc)
        : url_(url), description_(desc)
    { }
    ~UpdateImpl() throw() { }

    const char* url() const { return url_.c_str(); }

    const char* description() const { return description_.c_str(); }

    /**
     * @note may be useful to provide a "hook" for custom installs
     */
    void apply()
    {
        if (boost::python::object fun = this->get_override("apply"))
        {
            fun();
        }
    }

    Update* copy() const { return new UpdateImpl(*this); }
};


void export_update();

#endif // UPDATE_H__A99CB672_4EE5_42EA_A097_AF831CA33FB6
