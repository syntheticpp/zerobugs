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
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <iostream>
#include "pipe.h"
#include "system_error.h"

using namespace std;


Pipe::Pipe(bool nonblock)
{
    pipe_[0] = pipe_[1] = -1;

    for (;;)
    {
        if (::pipe(pipe_) == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            error("pipe open: ");
        }
        break;
    }
    if (nonblock)
    {
        fcntl(pipe_[0], F_SETFL, O_NONBLOCK);
        fcntl(pipe_[1], F_SETFL, O_NONBLOCK);
    }
    fcntl(pipe_[0], F_SETFD, FD_CLOEXEC);
    fcntl(pipe_[1], F_SETFD, FD_CLOEXEC);
}


Pipe::~Pipe() throw()
{
    if (pipe_[0] != -1)
    {
        close(pipe_[0]);
    }
    if (pipe_[1] != -1)
    {
        close(pipe_[1]);
    }
}


bool Pipe::write(const void* buf, size_t size, bool no_throw) volatile
{
    for (;;)
    {
        ssize_t n = ::write(pipe_[1], buf, size);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            error("pipe write: ", no_throw);
            return false;
        }
        if (n == 0)
        {
            return false;
        }
        fsync(pipe_[1]);
        break;
    }
    return true;
}


bool Pipe::read(void* buf, size_t size, bool no_throw)
{
    for (;;)
    {
        ssize_t n = ::read(pipe_[0], buf, size);

        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            error("pipe read", no_throw);
            return false;
        }
        break;
    }
    return true;
}


void Pipe::error(const char* str, bool no_throw) volatile
{
    SystemError err(str);

    if (no_throw)
    {
    #if DEBUG
        // cerr << err.what() << endl;
    #endif
    }
    else
    {
        throw err;
    }
}


void Pipe::close_input()
{
    if (pipe_[1] != -1)
    {
        close(pipe_[1]);
        pipe_[1] = -1;
    }
}


void Pipe::close_output()
{
    if (pipe_[0] != -1)
    {
        close(pipe_[0]);
        pipe_[0] = -1;
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
