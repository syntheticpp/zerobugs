//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: remote_ifstream.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <vector>
#include "remote_ifstream.h"
#include "rpc/remote_io.h"
#include "rpc/msg.h"
#include "rpc/rpc.h"

using namespace std;
using namespace RPC;


remote_ifstream_base::remote_ifstream_base(ObjectFactory& f, Stream& s, const char* filename)
{
    RefPtr<RemoteIO> msg(new RemoteIO(filename));
    rpc_channel rpc(f, s);
    rpc.send(__func__, msg);
    const int fd = value<rio_file>(*msg);
    buf_.reset(new remote_file_buf(rpc, fd));
}

remote_ifstream::remote_ifstream(ObjectFactory& f, Stream& s, const char* filename)
    : remote_ifstream_base(f, s, filename)
    , istream(buf_.get())
{
}


remote_ifstream::~remote_ifstream()
{
    if (buf_) buf_->close();
}


remote_file_buf::int_type remote_file_buf::underflow()
{
    assert(gptr() >= buffer_);
    assert(gptr() <= buffer_ + buffer_size);

    // read position before end of buffer?
    if (gptr() < egptr())
    {
        assert(gptr());
        return *gptr();
    }
    size_t nPutback = gptr() - eback();
    if (nPutback > putback_area_size)
    {
        nPutback = putback_area_size;
    }
    // copy up to putback_area_size characters previously read
    // into the putback buffer (area of first putback_area_size chars).
    assert(gptr() - nPutback >= buffer_);
    memcpy(buffer_ + (putback_area_size - nPutback), gptr() - nPutback, nPutback);

    // read more bytes from remote file
    vector<uint8_t> buf(buffer_size - putback_area_size);
    RefPtr<RemoteIO> msg(new RemoteIO(RIO_READ, &buf[0], buf.size() - 1));
    value<rio_file>(*msg) = fd_;
    rpc_.send(__func__, msg);
    const vector<uint8_t>& retbuf = value<rio_data>(*msg);
    if (retbuf.empty())
    {
        return EOF;
    }
    memcpy(buffer_ + putback_area_size, &retbuf[0], retbuf.size());

    // reset buffer pointers
    setg(buffer_ + (putback_area_size - nPutback),
         buffer_ + putback_area_size,
         buffer_ + putback_area_size + retbuf.size());

    return *gptr(); // return next character
}


void remote_file_buf::close()
{
    RefPtr<RemoteIO> msg(new RemoteIO(RIO_CLOSE, NULL, 0));
    value<rio_file>(*msg) = fd_;
    rpc_.send(__func__, msg);
    fd_ = -1;
}
