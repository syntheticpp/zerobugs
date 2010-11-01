//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: cstream.cpp 710 2010-10-16 07:09:15Z root $
//
// Implementation of character stream class CStream
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <errno.h>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "bstream.h"
#include "cstream.h"
#include "syscall_wrap.h"
#include "system_error.h"

using namespace std;

//
// helpers for read_opaque
//
template<typename T>
static vector<uint8_t> byte_buffer(const T& object)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&object);
    return vector<uint8_t>(ptr, ptr + sizeof(T));
}


template<typename T>
static void append(vector<uint8_t>& buffer, const T& object)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&object);
    buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}


CStream::CStream(auto_fd& fd, unsigned int wordSize) : Stream(wordSize), fd_(fd)
{
}


CStream::~CStream() throw()
{
}


/**
 * low level write
 */
size_t CStream::write_buffer(const void* buf, size_t count)
{
    sys::write(fd(), buf, count);
    return count;
}


/**
 * low level (raw) read
 */
size_t CStream::read_buffer(void* buf, size_t count)
{
    memset(buf, 0, count);
    for (;;)
    {
        ssize_t rc = ::read(fd(), buf, count);

        if (rc < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            throw SystemError(__func__);
        }
        else if (rc)
        {
            if (static_cast<size_t>(rc) != count)
            {
                ostringstream err;
                err << __func__ << ": expected " << count << ", got: " << rc;

                throw runtime_error(err.str());
            }
        }
        count = rc;
        break;
    }
    return count;
}



size_t CStream::write_object(const char* name, const Streamable* object)
{
    BStream bstream(word_size());
    size_t bytesWritten = bstream.write_object(name, object);
    assert(bytesWritten == bstream.buffer().size());

    return write_buffer(&bstream.buffer()[0], bytesWritten);
}


size_t CStream::read_object(const Descriptor& desc,
                            const char* name,
                            InputStreamEvents* events)
{
    size_t bytesRead = 0;

    ZDK_UUID uuid;
    bytesRead += read_buffer(&uuid, sizeof(uuid));

    if (!events)
    {
        return bytesRead;
    }
    size_t tmp = events->on_object_begin(this, name, &uuid, desc.size);
    if (tmp)
    {
        // assume that the callback has read the entire object
        bytesRead += tmp;
    }
    else if (desc.size)
    {
        vector<uint8_t> opaque = byte_buffer(desc);
        assert(opaque.size() == sizeof(desc));

        opaque.insert(opaque.end(),
                reinterpret_cast<const uint8_t*>(name),
                reinterpret_cast<const uint8_t*>(name + desc.namelen));
        assert(opaque.size() == (sizeof(desc) + desc.namelen));

        append(opaque, uuid);
        assert(opaque.size() == (desc.namelen + sizeof desc + sizeof uuid));

        const size_t where = opaque.size();
        opaque.resize(opaque.size() + desc.size);

        read_buffer(&opaque[where], desc.size);

        if (events)
        {
            events->on_opaque(name, &uuid, &opaque[0], opaque.size());
        }
        bytesRead += opaque.size();
    }
    return bytesRead;
}

