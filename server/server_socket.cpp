// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <errno.h>
#include <string.h> // for memset
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if DEBUG
 #include <iostream>
#endif
#include "dharma/system_error.h"
#include "server_socket.h"

using namespace std;


ServerSocket::ServerSocket(int s) : auto_fd(s)
{
}


ServerSocket::ServerSocket()
{
    reset(socket(AF_INET, SOCK_STREAM, 0));
    if (!is_valid())
    {
        throw SystemError("socket");
    }
}


void ServerSocket::bind(int port)
{
    linger nolinger = { 0, 0 };
    if (setsockopt(get(), SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger)) < 0)
    {
        throw SystemError("setsockopt");
    }
    sockaddr_in servAddr;

    memset(&servAddr, 0, sizeof (servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    if (::bind(get(), (const sockaddr*)&servAddr, sizeof (servAddr)) < 0)
    {
        throw SystemError("bind");
    }
}


void ServerSocket::listen(int backlog)
{
    if (::listen(get(), backlog) < 0)
    {
        throw SystemError("listen");
    }
}


auto_ptr<ServerSocket> ServerSocket::accept()
{
    sockaddr_in cliAddr;

    memset(&cliAddr, 0, sizeof (cliAddr));
    socklen_t addrLen = sizeof (cliAddr);

    auto_ptr<ServerSocket> s;

    while (s.get() == NULL)
    {
        int fd = ::accept(get(), (sockaddr*)&cliAddr, &addrLen);
        if (fd < 0)
        {
            if (errno != EINTR)
            {
                throw SystemError("accept");
            }
        }
        else
        {
            s.reset(new ServerSocket(fd));
        }
    }
#if DEBUG
    clog << __func__ << ": " << inet_ntoa(cliAddr.sin_addr) << endl;
#endif
    return s;
}
