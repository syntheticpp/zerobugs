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

#include <cassert>
#include <cerrno>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "zdk/byteswap.h"
#include "zdk/config.h"
#include "stream.h"
#include "system_error.h"
#include "syscall_wrap.h"


using namespace std;


OutputStreamImpl::OutputStreamImpl(unsigned int wordSize)
    : wordSize_(wordSize)
{
    assert(wordSize_ % 8 == 0);
}


OutputStreamImpl::~OutputStreamImpl() throw()
{
}


size_t OutputStreamImpl::write_word(const char* name, word_t val)
{
    assert(name);
    Descriptor desc;

    desc.type = (wordSize_ == 64) ? 'x' : 'i';
    desc.flags = 0;
    desc.namelen = strlen(name);
    desc.size = (wordSize_ / 8);

    write_buffer(&desc, sizeof(desc));
    write_buffer(name, desc.namelen);
    if (wordSize_ == 64)
    {
        int64_t i = val;
        write_buffer(&i, desc.size);
    }
    else
    {
        write_buffer(&val, desc.size);
    }
    return sizeof(desc) + desc.namelen + desc.size;
}


size_t OutputStreamImpl::write_double(const char* name, double val)
{
    assert(name);
    Descriptor desc;

    desc.type = 'd';
    desc.flags = 0;
    desc.namelen = strlen(name);
    desc.size = sizeof(double);

    write_buffer(&desc, sizeof(desc));
    write_buffer(name, desc.namelen);
    write_buffer(&val, desc.size);

    return sizeof(desc) + desc.namelen + desc.size;
}


size_t OutputStreamImpl::write_string(const char* name, const char* str)
{
    assert(name);
    assert(str);

    Descriptor desc;

    desc.type = 's';
    desc.flags = 0;
    desc.namelen = strlen(name);
    desc.size = strlen(str);

    write_buffer(&desc, sizeof(desc));
    write_buffer(name, desc.namelen);
    write_buffer(str, desc.size);

    return sizeof(desc) + desc.namelen + desc.size;
}


size_t OutputStreamImpl::write_bytes (const char* name, const void* bytes, size_t count)
{
    assert(name);
    assert(bytes || count == 0);

    Descriptor desc;

    desc.type = 'b';
    desc.flags = 0;
    desc.namelen = strlen(name);
    desc.size = count;

    write_buffer(&desc, sizeof(desc));
    write_buffer(name, desc.namelen);
    write_buffer(bytes, desc.size);

    return sizeof(desc) + desc.namelen + desc.size;
}


size_t OutputStreamImpl::write_opaque(const uint8_t* bytes, size_t size)
{
    // sanity-check the object
    if (size < sizeof(Descriptor))
    {
        throw runtime_error(string(__func__) + ": invalid object");
    }

    const Descriptor* desc = reinterpret_cast<const Descriptor*>(bytes);

    size_t calcSize = desc->size + desc->namelen + sizeof(*desc) + sizeof(ZDK_UUID);

    if ((desc->type != 'o') || (calcSize != size))
    {
        throw runtime_error(string(__func__) + ": Invalid object");
    }

    return write_buffer(bytes, size);
}


Stream::Stream(unsigned int wordSize) : OutputStreamImpl(wordSize)
{
}


Stream::~Stream() throw()
{
}


size_t Stream::read(InputStreamEvents* events)
{
    size_t bytesRead = 0;

    Descriptor desc = { 0, 0, 0, 0 };
    bytesRead += read_buffer(&desc, sizeof(desc));

    if (!bytesRead)
    {
        return 0;
    }

    vector<char> name(desc.namelen + 1);
    bytesRead += read_buffer(&name[0], desc.namelen);

    switch (desc.type)
    {
    case 0:
    #ifdef DEBUG
        clog << "null descriptor type\n";
    #endif
        break;

    case 'i':
        if (desc.size == sizeof(word_t))
        {
            word_t ival = 0;
            bytesRead += read_buffer(&ival, sizeof(ival));

            if (events)
            {
                events->on_word(&name[0], ival);
            }
        }
        else
        {
            throw runtime_error("read: invalid WORD");
        }
        break;

    case 'x':
        if (desc.size == sizeof(int64_t))
        {
        #if 0 && defined(DEBUG) && (__WORDSIZE < 64)
            // warn about possible data loss due to truncating 64bit integer
            clog << "Warning: possible truncation of 64-bit value\n";
        #endif
            int64_t ival = 0;
            bytesRead += read_buffer(&ival, sizeof(ival));

            if (events)
            {
                events->on_word(&name[0], ival);
            }
        }
        else
        {
            throw runtime_error("read: invalid int64");
        }
        break;

    case 'd':
        if (desc.size == sizeof(double))
        {
            double dval = 0;
            bytesRead += read_buffer(&dval, sizeof(dval));

            if (events)
            {
                events->on_double(&name[0], dval);
            }
        }
        else
        {
            throw runtime_error("read: invalid DOUBLE");
        }
        break;

    case 's':
        {
            vector<char> sval(desc.size + 1);

            bytesRead += read_buffer(&sval[0], desc.size);

            if (events)
            {
                events->on_string(&name[0], &sval[0]);
            }
        }
        break;

    case 'o':
        bytesRead += read_object(desc, &name[0], events);
        break;

    case 'b':
        {
            vector<uint8_t> bytes(desc.size);

            bytesRead += read_buffer(&bytes[0], desc.size);

            if (events)
            {
                events->on_bytes(&name[0], &bytes[0], desc.size);
            }
        }
        break;

    default:
        throw runtime_error("read: unknown descriptor");
    }
    return bytesRead;
}

