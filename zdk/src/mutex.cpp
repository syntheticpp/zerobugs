//
// $Id: mutex.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#ifndef _GNU_SOURCE
 #define _GNU_SOURCE
#endif
#include "zdk/config.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/time.h>   // for timespec
#include "zdk/mutex.h"

#define PTHREAD_ENFORCE(fun, ...) \
    while (int err = fun(__VA_ARGS__)) { throw_mutex_error(#fun, err); }


using namespace std;


/**
 * @note that we call this from some no-throw spots (such as dtors),
 * but this is because we really do not expect a pthread functions
 * to fail when releasing a resource; if there is an error, then
 * let terminate() do the dirty deed...
 */
static void throw_mutex_error(const string& func, int err)
{
    static bool haveThrown = false;
    if (haveThrown)
    {
        return;
    }
    haveThrown = true;
#ifdef DEBUG
    if (getenv("ZERO_ABORT_ON_MUTEX_ERROR"))
    {
        abort();
    }
#endif
    throw runtime_error(func + ": " + strerror(err));
}


CLASS MutexAttr : boost::noncopyable
{
    pthread_mutexattr_t attr_;

public:
    explicit MutexAttr(bool recursive)
    {
        PTHREAD_ENFORCE(pthread_mutexattr_init,(&attr_));
        if (recursive)
        {
            pthread_mutexattr_settype(&attr_, PTHREAD_MUTEX_RECURSIVE);
        }
    }
    ~MutexAttr()
    {
        pthread_mutexattr_destroy(&attr_);
    }
    operator const pthread_mutexattr_t* () const
    {
        return &attr_;
    }
};


Mutex::Mutex(bool recursive)
{
    MutexAttr attr(recursive);
    PTHREAD_ENFORCE(pthread_mutex_init, &mutex_, attr);
}


Mutex::~Mutex()
{
    // if macro throws from dtor it means things are FUBAR
    // since pthread_mutex_destroy should never fail here,
    // so let it terminate()
    PTHREAD_ENFORCE(pthread_mutex_destroy, &mutex_);
}


void Mutex::enter()
{
    PTHREAD_ENFORCE(pthread_mutex_lock, &mutex_);
    inc_lock_count();
}


void Mutex::leave() throw()
{
    dec_lock_count();
    PTHREAD_ENFORCE(pthread_mutex_unlock, &mutex_);
}


bool Mutex::leave(nothrow_t)
{
    if (pthread_mutex_unlock(&mutex_) == 0)
    {
        dec_lock_count();
        return true;
    }
    return false;
}


void Mutex::wait(pthread_cond_t& c)
{
    assert_lock_count();
    PTHREAD_ENFORCE(pthread_cond_wait, &c, &mutex_);
}


void Mutex::wait(volatile pthread_cond_t& c) volatile
{
    assert_lock_count();
    PTHREAD_ENFORCE(pthread_cond_wait,
            const_cast<pthread_cond_t*>(&c),
            const_cast<pthread_mutex_t*>(&mutex_));
}


void Mutex::wait(pthread_cond_t& c, long milliseconds)
{
    if (milliseconds == -1)
    {
        this->wait(c);
    }
    else
    {
        timeval now;
        gettimeofday(&now, NULL);

        unsigned long seconds = milliseconds / 1000;
        unsigned long nanoseconds = (milliseconds % 1000) * 1000000;

        timespec timeout = {
            now.tv_sec + seconds,
            now.tv_usec * 1000 + nanoseconds
        };

        if (int err = pthread_cond_timedwait(&c, &mutex_, &timeout))
        {
            if (err != ETIMEDOUT)
            {
                throw_mutex_error("pthread_cond_timedwait", err);
            }
        #ifdef DEBUG
            else
            {
                fprintf(stderr, "%s: timedout\n", __func__);
            }
        #endif
        }
    }
}



Condition::Condition()
{
    PTHREAD_ENFORCE(pthread_cond_init, &cond_, 0);
}


Condition::~Condition()
{
    PTHREAD_ENFORCE(pthread_cond_destroy, &cond_);
}


void Condition::notify() volatile
{
    PTHREAD_ENFORCE(pthread_cond_signal, const_cast<pthread_cond_t*>(&cond_));
}


void Condition::broadcast() volatile
{
    PTHREAD_ENFORCE(pthread_cond_broadcast, const_cast<pthread_cond_t*>(&cond_));
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
