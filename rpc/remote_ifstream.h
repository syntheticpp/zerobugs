#ifndef REMOTE_IFSTREAM_H__40B6D4EA_5365_49A8_80DD_372C4F783551
#define REMOTE_IFSTREAM_H__40B6D4EA_5365_49A8_80DD_372C4F783551

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <istream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <boost/shared_ptr.hpp>
#include "generic/auto_file.h"
#include "dharma/cstream.h"
#include "rpc/rpc.h"


CLASS rpc_channel
{
    ObjectFactory&  factory_;
    RPC::Stream&    stream_;    // for communication w/ server

public:
    rpc_channel(ObjectFactory& f, RPC::Stream& s) : factory_(f), stream_(s)
    { }

    template<typename T>
    void send(const char* func, RefPtr<T>& msg)
    {
        if (!RPC::send(factory_, func, stream_, msg))
        {
            throw std::runtime_error(func
                + std::string(": RPC communication error"));
        }
    }
};


/**
 * Uses RPC messages to read from a remote file.
 * @see RPC::Message
 * @see RPC::RemoteIO
 *
 * Thanks to Nicolai M. Josuttis for providing an example for
 * how to implement a custom streambuf in: The C++ Standard Library
 * Addison & Wesley 1999 (Chapter 13, page 678)
 */
CLASS remote_file_buf : public std::streambuf
{
    static const size_t buffer_size = 1024;
    static const size_t putback_area_size = 64;

    char        buffer_[buffer_size];
    rpc_channel rpc_;
    int         fd_;

protected:
    virtual int_type underflow();

public:
    remote_file_buf(rpc_channel& rpc, int fd) : rpc_(rpc), fd_(fd)
    {
        setg(buffer_ + putback_area_size,
             buffer_ + putback_area_size,
             buffer_ + putback_area_size);
    }
    ~remote_file_buf() { }
    void close();
    
    void seek(word_t offset);
};


CLASS remote_ifstream_base
{
protected:
    boost::shared_ptr<remote_file_buf> buf_;

    remote_ifstream_base(ObjectFactory& f, Stream& s, const char*);
};


/**
 * Provide an input stream abstraction that hides the details 
 * of the RCP required to read from a file on the remote target.
 */
CLASS remote_ifstream : remote_ifstream_base, public std::istream
{
public:
    remote_ifstream(ObjectFactory&, RPC::Stream&, const char* filename);

    ~remote_ifstream();

    // this is only intended to work in the following use-case:
    // open a stream
    // seek to some offset
    // read
    // close
    void seek(word_t offset)
    {
        buf_->seek(offset);
    }
};


#endif // REMOTE_IFSTREAM_H__40B6D4EA_5365_49A8_80DD_372C4F783551
