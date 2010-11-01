// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: redirect.cpp 710 2010-10-16 07:09:15Z root $
//
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "dharma/system_error.h"
#include "redirect.h"

using namespace std;
using namespace boost;


typedef std::vector<shared_ptr<Redirect::Undo> > RedirectStack;

static bool redirectForkHandlersSet = false;
static RedirectStack& redirect_stack()
{
    static RedirectStack redirectStack;
    return redirectStack;
}

RedirectStack* forceInit = &redirect_stack();

static void undo_all_redirects()
{
#ifdef DEBUG
    clog << "clearing " << redirect_stack().size() << " redirect(s)\n";
#endif
    redirect_stack().clear();
}



Redirect::Undo::Undo(int fd)
    : fd_(fd)
    , dup_(dup(fd))
{
    if (!dup_.is_valid())
    {
        throw SystemError("redirect: dup");
    }
}


Redirect::Undo::~Undo()
{
    dup2(dup_.get(), fd_);
}


Redirect::Redirect(int fd, auto_fd& file) : dest_(-1), undo_(0)
{
    init(fd, file.get());
}


Redirect::Redirect(int fd, int file, bool autoClose)
    : dest_(autoClose ? file : -1)
    , undo_(0)
{
    init(fd, file);
}


void Redirect::init(int fd, int file)
{
    if (!redirectForkHandlersSet)
    {
        pthread_atfork(NULL, NULL, undo_all_redirects);
        redirectForkHandlersSet = true;
    }
    shared_ptr<Undo> undo(new Undo(fd));
    undo_ = undo.get();
    redirect_stack().push_back(undo);

    if (file >= 0)
    {
        if (dup2(file, fd) < 0)
        {
            throw SystemError("redirect: dup2");
        }
    }
}


Redirect::~Redirect()
{
    assert (!redirect_stack().empty());

    if (!redirect_stack().empty())
    {
        assert(redirect_stack().back().get() == undo_);
        redirect_stack().pop_back();
    }
    if (dest_ >= 0)
    {
        ::close(dest_);
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
