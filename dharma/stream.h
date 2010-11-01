#ifndef STREAM_H__652310AA_14A2_41E2_B9CE_37353C1719B5
#define STREAM_H__652310AA_14A2_41E2_B9CE_37353C1719B5
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: stream.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/stream.h"
#include "generic/auto_file.h"
#include <boost/static_assert.hpp>


struct ZDK_LOCAL Descriptor
{
    int8_t      type;
    int8_t      flags;
    uint16_t    namelen;
    uint32_t    size;
};
BOOST_STATIC_ASSERT(sizeof(Descriptor) == 8);

enum Flags
{
    F_NONE,
    F_BIGENDIAN,
};


CLASS OutputStreamImpl : public OutputStream
{
    // non-copyable, non-assignable
    OutputStreamImpl(const OutputStreamImpl&);
    OutputStreamImpl& operator=(const OutputStreamImpl&);

    unsigned int wordSize_;

public:
    virtual ~OutputStreamImpl() throw();

    /*** OutputStream interface ***/
    virtual size_t write_word(const char* name, word_t val);

    virtual size_t write_double(const char* name, double val);

    virtual size_t write_string(const char*, const char*);

    virtual size_t write_opaque(const uint8_t*, size_t);

    virtual size_t write_bytes(const char*, const void*, size_t);

    // derived classes need to implement this method
    // virtual size_t write_object(const char*, const Streamable*);

protected:
    explicit OutputStreamImpl(unsigned int wordSize);

    /**
     * low level write
     */
    virtual size_t write_buffer(const void* buf, size_t count) = 0;

    unsigned int word_size() const { return wordSize_; }
};


/**
 * Base stream implementation.
 * A stream supports the serialization of a Streamable object.
 */
CLASS Stream : public OutputStreamImpl, public InputStream
{
    // non-copyable, non-assignable
    Stream(const Stream&);
    Stream& operator=(const Stream&);

public:
    virtual ~Stream() throw();

    /*** InputStream interface ***/
    virtual size_t read(InputStreamEvents*);

protected:
    explicit Stream(unsigned int wordSize);

    /**
     * low-level, read unstructured bytes
     */
    virtual size_t read_buffer(void* buf, size_t count) = 0;

    virtual size_t read_object(const Descriptor&,
                               const char* name,
                               InputStreamEvents*) = 0;
};

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
#endif // STREAM_H__652310AA_14A2_41E2_B9CE_37353C1719B5
