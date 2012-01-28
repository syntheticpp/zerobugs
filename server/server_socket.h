#ifndef SERVER_SOCKET_H__D68A9346_C2C3_4C97_9C5A_EA94017CBA45
#define SERVER_SOCKET_H__D68A9346_C2C3_4C97_9C5A_EA94017CBA45
//
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
//
#include <memory>
#include "generic/auto_file.h"


CLASS ServerSocket : public auto_fd
{
public:
    ServerSocket();
    explicit ServerSocket(int);

    void listen(int backlog = 5);

    void bind(int port);

    std::auto_ptr<ServerSocket> accept();
};

#endif // SERVER_SOCKET_H__D68A9346_C2C3_4C97_9C5A_EA94017CBA45
