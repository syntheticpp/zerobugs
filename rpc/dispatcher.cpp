//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: dispatcher.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <iostream>
#include <stdexcept>
#include "dispatcher.h"

using namespace std;
using namespace RPC;


Dispatcher::~Dispatcher() throw()
{
}


void Dispatcher::on_word(const char* name, word_t val)
{
    set_word(name, val);
}


void Dispatcher::on_double(const char* name, double val)
{
    set_double(name, val);
}


void Dispatcher::on_string(const char* name, const char* val)
{
    set_string(name, val);
}


template<typename T> static void
handle_exception(const T& e, OutputStream& output, bool isServerSide)
{
    if (!isServerSide)
    {
        throw;
    }
    else
    {
    #if DEBUG
        clog << __func__ << ": " << e.what() << endl;
    #endif
        try
        {
            Error err(e);
            output.write_object("error", &err);
        }
        catch (...)
        {
            throw e;
        }
    }
}


static bool
dispatch(Message& msg, InputStream& input, OutputStream& output, bool isServerSide)
{
    bool result = true;
    try
    {
        result = msg.handle(input, output);
    }
    catch (const SystemError& e)
    {
        handle_exception(e, output, isServerSide);
    }
    catch (const exception& e)
    {
        handle_exception(e, output, isServerSide);
    }
    return result;
}


size_t
Dispatcher::on_object_begin(InputStream* input, const char* name, uuidref_t iid, size_t size)
{
    assert(input);

    size_t bytesRead = 0;

    if (RefPtr<ZObject> obj = factory_.create_object(iid, name))
    {
        response_ = obj;
        set_object(name, obj.get()); // save into dictionary

        if (RefPtr<Message> msg = interface_cast<Message>(obj))
        {
            while (bytesRead < size)
            {
                bytesRead += input->read(msg.get());
            }
        #if 0 // DEBUG
            clog << __func__ << ": " << name << ": " << bytesRead << " bytes read\n";
        #endif
            if (interface_cast<Error>(obj))
            {
                dispatch(*msg, *input, output_, is_server_side());
            }
            message_ = msg;
        }
        else
        {
            clog << "not a RPC message: " << name << endl;
        }
    }
    else
    {
        char uuid[37];
        uuid_to_string(iid, uuid);

        throw runtime_error("interface not registered with factory: " + string(uuid));
    }
    return bytesRead;
}


void Dispatcher::on_bytes(const char* name, const void*, size_t)
{
}


void Dispatcher::on_opaque(const char* name, uuidref_t iid, const uint8_t*, size_t)
{
#if DEBUG
    char uuid[37];
    uuid_to_string(iid, uuid);

    clog << __func__ << ": " << name << " " << uuid << endl;
#endif
}


bool Dispatcher::read_and_dispatch_msg(InputStream& input)
{
    message_.reset();

    while (input.read(this) && !message_)
    { }
    //clog << "message=" << message_.get() << endl;

    if (message_)
    {
        return dispatch(*message_, input, output_, is_server_side());
    }
    return false;
}
