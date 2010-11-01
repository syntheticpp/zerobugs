// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "bstream.h"
#include <stdexcept>

using namespace std;


BStream::BStream(unsigned int wordSize) : OutputStreamImpl(wordSize)
{
}


BStream::~BStream() throw()
{
}


size_t BStream::write_object(const char* name, const Streamable* object)
{
    assert(name);
    assert(object);

    Descriptor desc;

    desc.type = 'o';
    desc.flags = 0;
    desc.namelen = strlen(name);
    desc.size = 0xFE;

    write_buffer(&desc, sizeof(desc));
    write_buffer(name, desc.namelen);
    write_buffer(object->uuid(), sizeof(ZDK_UUID));

    desc.size = object->write(this);

    const size_t skip = desc.size + sizeof(ZDK_UUID) + desc.namelen;
    const loff_t offs = skip + sizeof(desc.size);

    assert(buf_.size() >= static_cast<size_t>(offs));

    uint32_t& x = *(uint32_t*)&buf_[buf_.size() - offs];

    // make sure that the file position is at the object's size
    assert(x == 0xFE);
    x = desc.size;

    return sizeof(desc) + desc.namelen + sizeof(ZDK_UUID) + desc.size;
}


size_t BStream::write_buffer(const void* buf, size_t count)
{
    const size_t oldSize = buf_.size();
    buf_.resize(oldSize + count);

    memcpy(&buf_[oldSize], buf, count);
    return count;
}

