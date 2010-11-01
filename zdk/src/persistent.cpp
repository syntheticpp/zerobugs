//
// $Id: persistent.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <stdexcept>
#include <string>
#include "zdk/persistent.h"



Persistent::Persistent(const char* name)
{
    assert(name);
    name_.assign(name);
}


Persistent::~Persistent() throw()
{
}


void Persistent::error(const char* name, const char* type)
{
    std::string msg(name_);

    msg += " has no member `";
    msg += name;
    msg += "' of type ";
    msg += type;

    throw std::runtime_error(msg);
}


void Persistent::on_word(const char* name, word_t)
{
    error(name, "int");
}


void Persistent::on_double(const char* name, double)
{
    error(name, "double");
}


void Persistent::on_string(const char* name, const char*)
{
    error(name, "string");
}


void Persistent::on_opaque(const char* name, uuidref_t, const uint8_t*, size_t)
{
    error(name, "opaque");
}


size_t Persistent::on_object_begin(InputStream*,
                                   const char* name,
                                   uuidref_t uuid,
                                   size_t)
{
    char uuidstr[37];

    uuid_to_string(uuid, uuidstr);

    error(name, uuidstr);
    return 0;
}


void Persistent::on_bytes(const char* name, const void*, size_t)
{
    error(name, "opaque");
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
