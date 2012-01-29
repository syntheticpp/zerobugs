#ifndef MUTEX_H__CA16EB19_D743_48A0_B58B_1C5AB614BBB6
#define MUTEX_H__CA16EB19_D743_48A0_B58B_1C5AB614BBB6
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
#include "zdk/stdexcept.h"
#if WIN32
 #include "zdk/win/mutex.h"
#else

#include <pthread.h>
#include <new> // nothrow_t
#include <boost/utility.hpp>
#include "zdk/atomic.h"
#include "zdk/export.h"
#include "generic/lock.h"

class Condition;

/**
 * C++ wrapper for pthreads mutexes
 */
class ZDK_LOCAL Mutex : boost::noncopyable
{
#if defined(__GNUC__) && (__GNUC__ < 4)
    //
    // HAVE_TEMPLATE_FRIEND_METHODS
    //
    friend class Lock<Mutex>;
#else
    template<typename C> friend void Lock<Mutex>::wait(C&, long);
#endif

public:
    explicit Mutex(bool recursive = true);
    virtual ~Mutex();

    void enter();
    void leave() throw();
    bool leave(std::nothrow_t);

    bool trylock();

    void assert_locked() volatile { assert_lock_count(); }

    pthread_t get_owner() const volatile
    {
        return owner_;
    }

#ifdef DEBUG
    // return source file name and line where acquired
    const char* file() const volatile
    {
        return file_;
    }
    size_t line() const volatile
    {
        return line_;
    }
    void set_file_attempt(const char* file) 
    {
        fileAttempt_ = file;
    }
    void set_line_attempt(size_t line)
    {
        lineAttempt_ = line;
    }
#endif // DEBUG

protected:
    void wait(pthread_cond_t&);
    void wait(volatile pthread_cond_t&) volatile;
    void wait(pthread_cond_t&, long milliseconds);

private:
    void set_owner();

    pthread_mutex_t mutex_;
    pthread_t owner_;

#ifdef DEBUG
    atomic_t lock_;
    const char* fileAttempt_;
    size_t lineAttempt_;
    const char* file_;
    size_t line_;
    
    void inc_lock_count() { atomic_inc(lock_); }
    void dec_lock_count() { atomic_dec(lock_); }
    //void init_lock_count() { atomic_set(lock_, 0); }
    void assert_lock_count() volatile { assert (atomic_read(lock_)); }
#else
    static void inc_lock_count() { }
    static void dec_lock_count() { }
    static void init_lock_count() { }
    static void assert_lock_count() { }
#endif
};


class ZDK_LOCAL Condition : boost::noncopyable
{
    pthread_cond_t cond_;

    template<typename C> friend void Lock<Mutex>::wait(C&, long);

public:
    Condition();
    ~Condition();

    void notify() volatile; // todo: rename to notify_one for boost::thread compat
    void broadcast() volatile; // todo: rename to notify_all

    int broadcast(std::nothrow_t) throw()
    {
        return pthread_cond_broadcast(&cond_);
    }

    void wait(Lock<Mutex>& lock, long timeout = -1)
    {
        lock.wait(cond_, timeout);
    }
};
#endif
#endif // MUTEX_H__CA16EB19_D743_48A0_B58B_1C5AB614BBB6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
