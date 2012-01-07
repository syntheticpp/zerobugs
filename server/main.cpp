// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: main.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "server.h"
#include "forked_server.h"
#include "socket_base_server.h"

using namespace std;

typedef Server<ForkedServer<SocketBaseServer> > DebugServer;


int main(int argc, char* argv[])
{
    cout << "*** ZeroBUGS Remote Debug Server V. 1.0 ***" << endl;

    try
    {
        DebugServer server;

        server.init(argc, argv);
        server.main_loop();
    }
    catch (const exception& e)
    {
        cerr << __func__ << ": " << e.what() << endl;
        return 1;
    }
    return 0;
}

