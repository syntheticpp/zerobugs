#ifndef FORKED_SERVER_H__27F2A32B_B490_4D18_BBAB_AE2CCC8983B4
#define FORKED_SERVER_H__27F2A32B_B490_4D18_BBAB_AE2CCC8983B4
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
#include <assert.h>
#include <signal.h>
#include <iostream>
#include <stdexcept>
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "dharma/cstream.h"
#include "dharma/syscall_wrap.h"


template<typename T, typename S = CStream>
CLASS ForkedServer : public T
{
    static ext::hash_set<pid_t> children_;

    static void sigchld(int, siginfo_t* sinfo, void*)
    {
        assert(sinfo);
        assert(sinfo->si_signo == SIGCHLD);
        int status = 0;
        ext::hash_set<pid_t>::iterator i = children_.find(sinfo->si_pid);

        switch (sinfo->si_code)
        {
        case CLD_EXITED:
        case CLD_KILLED:
        case CLD_DUMPED:
        #if DEBUG
            std::clog << __func__<< ": si_pid=" << sinfo->si_pid;
            std::clog << " si_code=" << sinfo->si_code << std::endl;
        #endif
            if (i != children_.end())
            {
                children_.erase(i);
                waitpid(sinfo->si_pid, &status, 0);
            }
            break;
        }
    }

public:
    //the communication channel type
    typedef typename T::channel_type channel_type;
    typedef S stream_type;

    ForkedServer()
    {
        struct sigaction act;
        memset(&act, 0, sizeof act);

        act.sa_sigaction = sigchld;
        act.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
        sigaction(SIGCHLD, &act, 0);
    }

    virtual bool handle_client_msg(stream_type&) = 0;

    void main_loop()
    {
        for (T::listen();;)
        {
            try
            {
                std::auto_ptr<channel_type> channel = T::accept();

            #if DEBUG
                std::clog << __func__ << ": connection accepted\n";
            #endif
                assert(channel.get());
                assert(channel->is_valid());

                pid_t pid = sys::fork();
                if (pid)
                {
                    children_.insert(pid); // parent
                }
                else
                {
                    T::close();
                    sys::unmask_all_signals();

                    stream_type s(*channel);

                    while (handle_client_msg(s))
                    {
                    }
                    break;
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << __func__ << ": " << e.what() << std::endl;
            }
        }
    #ifdef DEBUG
        std::clog << __func__ << ": done, pid=" << getpid() << std::endl;
    #endif
    }
};


template<typename T, typename S>
    ext::hash_set<pid_t> ForkedServer<T, S>::children_;

#endif // FORKED_SERVER_H__27F2A32B_B490_4D18_BBAB_AE2CCC8983B4
