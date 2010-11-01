// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: flock.cpp 714 2010-10-17 10:03:52Z root $
//
#include <assert.h>
#if DEBUG
 #include <iostream>
#endif
#include "flock.h"
#include "system_error.h"

using namespace std;


FileLock::FileLock(Type type, int fd) : type_(type), fd_(fd)
{
    if (flock(fd_, type_) < 0)
    {
        throw SystemError("flock");
    }
}


FileLock::~FileLock()
{
    assert(fd_ >= 0);
    flock(fd_, LOCK_UN);
}


ExclusiveFileLock::ExclusiveFileLock(int fd)
    : FileLock(lock_excl, fd)
{
}


ExclusiveFileLock::ExclusiveFileLock(int fd, nothrow_t nothrow)
    : FileLock(lock_excl, fd, nothrow)
{
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
