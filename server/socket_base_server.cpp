// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: socket_base_server.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <unistd.h>
#include <iostream>
#include "zdk/argv_util.h"
#include "rpc/msg.h"
#include "socket_base_server.h"


SocketBaseServer::~SocketBaseServer()
{
}


void SocketBaseServer::init(int& argc, char**& argv)
{
    int port = RPC::port;

    BEGIN_ARG_PARSE(&argc, &argv)

        ON_ARGV("--port=", port)
        {
    #if DEBUG
            std::clog << __func__ << ": port=" << port<< std::endl;
    #endif
        }
        ON_ARG("--foregnd")
        {
            foregnd_ = true;
        }
    END_ARG_PARSE

    socket_.reset(new ServerSocket());
    socket_->bind(port);

    if (!foregnd_)
    {
        daemon(1, 0);
    }
}
