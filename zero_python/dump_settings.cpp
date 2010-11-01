//
// $Id: dump_settings.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <iostream>
#include "dump_settings.h"

using namespace std;
using namespace boost::python;



DumpSettings::DumpSettings()
{
}


DumpSettings::~DumpSettings() throw()
{
}


void DumpSettings::on_word(const char* name, word_t val)
{
    list_.append(make_tuple(name, "word", val));
}


void DumpSettings::on_double(const char* name, double val)
{
    list_.append(make_tuple(name, "double", val));
}


void DumpSettings::on_string(const char* name, const char* val)
{
    list_.append(make_tuple(name, "string", val));
}


size_t DumpSettings::on_object_begin(
    InputStream*    stream,
    const char*     name,
    uuidref_t       uuid,
    size_t          size)
{
    char buf[37];
    uuid_to_string(uuid, buf);

    DumpSettings obj;

    size_t result = 0;
    while (size_t nread = stream->read(&obj))
    {
        result += nread;
        if (result == size)
        {
            break;
        }
    }
    list_.append(make_tuple(name, buf, obj.list()));
    return result;
}


void DumpSettings::on_opaque(const char*, uuidref_t, const uint8_t*, size_t size)
{
    assert(false);
}


void DumpSettings::on_bytes(const char*, const void*, size_t size)
{
    assert(false);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
