#ifndef CSTREAM_H__6070F670_1201_45E3_824F_50C6187D0742
#define CSTREAM_H__6070F670_1201_45E3_824F_50C6187D0742

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "dharma/stream.h"
#include "generic/auto_file.h"


/**
 * Character, serial stream. The underlying file descriptor
 * may not support random access (basically we cannot lseek).
 */
CLASS CStream : public Stream
{
public:
    /// construct stream and attach it to open file;
    /// the Stream does not own the file.
    explicit CStream(auto_fd& fd, unsigned int wordSize = __WORDSIZE);

    virtual ~CStream() throw();

    virtual size_t write_object(const char*, const Streamable*);
    virtual size_t read_object(const Descriptor&,
                               const char* name,
                               InputStreamEvents*);

protected:
    virtual size_t write_buffer(const void* buf, size_t count);
    virtual size_t read_buffer(void* buf, size_t count);

    int fd() const { return fd_.get(); }

private:
    auto_fd& fd_;
};
#endif // CSTREAM_H__6070F670_1201_45E3_824F_50C6187D0742
