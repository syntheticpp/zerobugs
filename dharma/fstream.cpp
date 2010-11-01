//
// $Id: fstream.cpp 723 2010-10-28 07:40:45Z root $
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
#include <sys/fcntl.h>
#include <iostream>
#include "fstream.h"
#include "syscall_wrap.h"
#include "system_error.h"


using namespace std;


FileStream::FileStream(const char* fname, bool readOnly, unsigned wordSize)
    : CStream(fd_, wordSize)
{
    assert(fname);

    if (readOnly)
    {
        fd_.reset(open(fname, O_RDONLY, 0644));
    }
    else
    {
        fd_.reset(open(fname, O_CREAT | O_RDWR, 0644));
    }
    if (!fd_)
    {
        throw SystemError(fname);
    }
}


FileStream::~FileStream() throw()
{
}




void FileStream::truncate()
{
    for (;;)
    {
        if (lseek(fd(), 0, SEEK_SET) < 0
         || ftruncate(fd(), 0) < 0)
        {
            if (errno != EINTR)
            {
                throw SystemError(__func__);
            }
        }
        else
        {
            break;
        }
    }
}


size_t FileStream::size() const
{
    struct stat st;
    if (!fd_)
    {
        return 0;
    }
    if (fstat(fd(), &st) < 0)
    {
        throw SystemError(__func__);
    }
    return st.st_size;
}


loff_t FileStream::position() const
{
    if (!fd_)
    {
        return 0;
    }
    if (fsync(fd()) < 0)
    {
        throw SystemError(__func__);
    }
    return sys::lseek(fd(), SEEK_CUR, 0);
}


size_t FileStream::write_object(const char* name, const Streamable* object)
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

    off_t offs = skip + sizeof(desc.size);
    offs = lseek(fd(), -offs, SEEK_CUR);

#ifndef NDEBUG
    // make sure that the file position is at the object's size
    {
        uint32_t x = 0;
        read_buffer(&x, sizeof x);

        assert(x == 0xFE);
        lseek(fd(), -(off_t)sizeof(x), SEEK_CUR);
    }
#endif // DEBUG
    if (offs == (off_t) -1)
    {
        throw SystemError(__func__);
    }

    write_buffer(&desc.size, sizeof(desc.size));

    offs = lseek(fd(), skip, SEEK_CUR);

    if (offs == (off_t) -1)
    {
        throw SystemError(__func__);
    }
    return sizeof(desc) + desc.namelen + sizeof(ZDK_UUID) + desc.size;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
