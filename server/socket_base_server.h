#ifndef SOCKET_BASE_SERVER_H__C9A7EC54_4433_4D35_9007_A1D7C9EF8046
#define SOCKET_BASE_SERVER_H__C9A7EC54_4433_4D35_9007_A1D7C9EF8046
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: socket_base_server.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <cassert>
#include <boost/utility.hpp>
#include "server_socket.h"


CLASS SocketBaseServer : boost::noncopyable
{
    std::auto_ptr<ServerSocket> socket_;
    bool foregnd_;

public:
    typedef ServerSocket channel_type;

    SocketBaseServer() : foregnd_(false) { }
    virtual ~SocketBaseServer();

    void init(int& argc, char**& argv);

protected:
    void listen()
    {
        assert(socket_.get());
        socket_->listen();
    }

    std::auto_ptr<ServerSocket> accept()
    {
        assert(socket_.get());
        return socket_->accept();
    }

    void close()
    {
        socket_.reset();
    }

    bool foregnd() const { return foregnd_; }
};


#endif // SOCKET_BASE_SERVER_H__C9A7EC54_4433_4D35_9007_A1D7C9EF8046
