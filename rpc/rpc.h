#ifndef RPC_H__04E4E4EB_66F5_434A_A69C_81B8B8FE7247
#define RPC_H__04E4E4EB_66F5_434A_A69C_81B8B8FE7247
//
// $Id: rpc.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "dharma/stream.h"
#include "rpc/dispatcher.h"



namespace RPC
{
    using ::Stream;

    /**
     * Send message from client to remote debug server.
     * Read response from server and return it.
     */
    template<typename T>
    ZDK_LOCAL inline
    bool send(ObjectFactory& f, const char* name, Stream& stream, RefPtr<T>& msg)
    {
        stream.write_object(name, msg.get());
        Dispatcher dispatcher(f, stream, ClientSide());

        RefPtr<T> response;

        while (!response && stream.read(&dispatcher))
        {
            response = interface_cast<T>(dispatcher.response());
            msg = response;
        }
        return response;
    }

 /*
    template<AttrEnum attr>
    ZDK_LOCAL inline
    word_t send(ObjectFactory& f,
                const char* name,
                Stream& stream,
                const Message& msg)
    {
        stream.write_object(name, &msg);
        Dispatcher dispatcher(f, stream, ClientSide());

        stream.read(&dispatcher);
        const word_t result = dispatcher.get_word(attr_name[attr], 0);

        return result;
    } */
}
#endif // RPC_H__04E4E4EB_66F5_434A_A69C_81B8B8FE7247
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
