#ifndef FLOCK_H__7A9A830B_C6A8_41FE_B7D5_E1A8D794E9D8
#define FLOCK_H__7A9A830B_C6A8_41FE_B7D5_E1A8D794E9D8
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

#include <sys/file.h>

class FileLock
{
    // disable copy and assignment to prevent double unlocks
    FileLock(const FileLock&);
    FileLock& operator=(const FileLock&);

public:
    enum Type
    {
        Lock_shared = LOCK_SH,
        lock_excl   = LOCK_EX,
        lock_excl_nonblock = LOCK_EX | LOCK_NB
    };

    virtual ~FileLock();

    FileLock(Type, int fd);

private:
    const Type type_;
    int fd_;
};



class ExclusiveFileLock : public FileLock
{
public:
    explicit ExclusiveFileLock(int fd);
};



#endif // FLOCK_H__7A9A830B_C6A8_41FE_B7D5_E1A8D794E9D8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
