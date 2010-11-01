#ifndef BSTREAM_H__4AF83C9D_AA6A_4E58_8A5A_9B9236A9C328
#define BSTREAM_H__4AF83C9D_AA6A_4E58_8A5A_9B9236A9C328
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: bstream.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <vector>
#include "stream.h"


CLASS BStream : public OutputStreamImpl
{
public:
    explicit BStream(unsigned int wordSize);
    virtual ~BStream() throw();

    virtual size_t write_object(const char*, const Streamable*);

    const std::vector<uint8_t>& buffer() const { return buf_; }

protected:
    virtual size_t write_buffer(const void* buf, size_t count);

private:
    std::vector<uint8_t> buf_;
};
#endif // BSTREAM_H__4AF83C9D_AA6A_4E58_8A5A_9B9236A9C328
