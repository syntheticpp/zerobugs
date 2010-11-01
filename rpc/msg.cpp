//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: msg.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdexcept>
#include "msg.h"

using namespace std;



RPC::Message::~Message() throw()
{
}


void RPC::Message::on_word(const char* name, word_t)
{
    assert(name);
    throw runtime_error(string(__func__) + ": unexpected \"" + name + "\"");
}


void RPC::Message::on_double(const char* name, double)
{
    assert(name);
    throw runtime_error(string(__func__) + ": unexpected \"" + name + "\"");
}


void RPC::Message::on_string(const char* name, const char*)
{
    assert(name);
    throw runtime_error(string(__func__) + ": unexpected \"" + name + "\"");
}


size_t RPC::Message::on_object_begin(
        InputStream*,
        const char* name,
        uuidref_t   uuid,
        size_t      size)
{
    assert(name);
    throw runtime_error(string(__func__) + ": unexpected \"" + name + "\"");
}


void RPC::Message::on_object_end()
{
}


void RPC::Message::on_opaque(const char* name, uuidref_t, const uint8_t*, size_t)
{
    assert(name);
    throw runtime_error(string(__func__) + ": unexpected \"" + name + "\"");
}


void RPC::Message::on_bytes(const char* name, const void*, size_t)
{
    assert(name);
    throw runtime_error(string(__func__) + ": unexpected \"" + name + "\"");
}


RPC::Error::~Error() throw()
{
}


RPC::Exec::~Exec() throw()
{
}


RPC::Kill::~Kill() throw()
{
}

RPC::SysInfo::~SysInfo() throw()
{
}


RPC::Ptrace::~Ptrace() throw()
{
}


RPC::Waitpid::~Waitpid() throw()
{
}


RPC::RemoteIO::~RemoteIO() throw()
{
}
