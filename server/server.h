#ifndef SERVER_H__F1D27F65_AB96_42B2_9FE9_BE62E46CB8D1
#define SERVER_H__F1D27F65_AB96_42B2_9FE9_BE62E46CB8D1
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
#include "rpc/dispatcher.h"
#include "factory.h"


/**
 * Generic server, dispatches client requests.
 * Uses a Factory to instantiate objects on the server side.
 *
 * The template parameter B allows to inject a base class that
 * implements other server-side policies (such us how to handle
 * concurrent requests). Template parameter S is a place holder
 * for the actual communication stream between the client and the
 * server.
 */
template<typename B, typename S = typename B::stream_type>
CLASS Server : public B
{
    Factory factory_;

public:
    Server()
    {
    }

    bool handle_client_msg(S& s)
    {
        RPC::Dispatcher d(factory_, s, RPC::ServerSide());
        return d.read_and_dispatch_msg(s);
    }
};

#endif // SERVER_H__F1D27F65_AB96_42B2_9FE9_BE62E46CB8D1
