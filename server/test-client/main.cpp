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

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <boost/lexical_cast.hpp>
#include "dharma/cstream.h"
#include "dharma/object_factory.h"
#include "dharma/system_error.h"
#include "generic/auto_file.h"
#include "rpc/msg.h"
#include "rpc/rpc.h"
#include "rpc/remote_ifstream.h"
#include "rpc/remote_io.h"
#include "zdk/zero.h"


using namespace std;


static void
connect(const string& hostname, sockaddr_in& servAddr, auto_fd& sock)
{
    if (inet_aton(hostname.c_str(), &servAddr.sin_addr) == 0) // not an addr?
    {
        if (const hostent* entry = gethostbyname(hostname.c_str()))
        {
            memcpy(&servAddr.sin_addr, entry->h_addr, sizeof(in_addr));
        }
        else
        {
            throw runtime_error(hstrerror(h_errno));
        }
    }
    if (::connect(sock.get(), (const sockaddr*)&servAddr, sizeof (servAddr)) < 0)
    {
        throw SystemError("connect");
    }
}


/**
 * Connect to the remote server.
 * @return the path to the remote executable.
 */
static string connect(const string& s, auto_fd& sock)
{
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof (servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(RPC::port);

    string hostname, result;

    size_t hostEnd = s.find('/', 0);
    if (hostEnd == string::npos)
    {
        hostname = s.substr(0);
    }
    else
    {
        result = s.substr(hostEnd);
        hostname = s.substr(0, hostEnd);
    }
    cout << "hostname=" << hostname << endl;
    hostEnd = hostname.find(':'); // look for port number
    if (hostEnd != string::npos)
    {
        const char* port = hostname.c_str() + hostEnd + 1;
        servAddr.sin_port = boost::lexical_cast<int>(port);
        servAddr.sin_port = htons(servAddr.sin_port);
        hostname.resize(hostEnd);
    }
    connect(hostname, servAddr, sock);
    return result;
} // connect



int main(int argc, char* argv[])
{
    try
    {
        auto_fd sock(socket(AF_INET, SOCK_STREAM, 0));
        ObjectFactoryImpl f(NULL);

        for (--argc, ++argv; argc; --argc, ++argv)
        {
            string filename = connect(*argv, sock);
            cout << "filename=" << filename << endl;
            CStream cstream(sock);
        /*
            const uint8_t* bytes = (const uint8_t*)filename.c_str();
            RefPtr<RPC::RemoteIO> msg(
                    new RPC::RemoteIO(RPC::RIO_OPEN, bytes, filename.size() + 1));

            if (!RPC::send(f, __func__, cstream, msg))
                throw runtime_error("open: send failed");
            const int fd = RPC::value<RPC::rio_file>(*msg);
            cout << "file descriptor=" << fd << endl;

            do
            {
                vector<uint8_t> buf(256);
                msg.reset(new RPC::RemoteIO(RPC::RIO_READ, &buf[0], buf.size() - 1));
                RPC::value<RPC::rio_file>(*msg) = fd;
                if (!RPC::send(f, __func__, cstream, msg))
                    throw runtime_error("read: send failed");

                buf = RPC::value<RPC::rio_data>(*msg);
                cout << &buf[0];
            } while (RPC::value<RPC::rio_ret>(*msg));
            cout << endl;

            msg.reset(new RPC::RemoteIO(RPC::RIO_CLOSE, NULL, 0));
            RPC::value<RPC::rio_file>(*msg) = fd;
            if (!RPC::send(f, __func__, cstream, msg))
                throw runtime_error("close: send failed");
         */
            remote_ifstream ifs(f, cstream, filename.c_str());
            char line[4096] = { 0 } ;
            while (ifs.getline(line, sizeof line))
            {
                cout << line << endl;
                memset(line, 0, sizeof line);
            }
        }
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
        return -1;
    }
    return 0;
}
