#ifndef THREAD_SAFE_QUEUE_H__F6C0A216_8C18_45E4_96DD_10D6F990DD27
#define THREAD_SAFE_QUEUE_H__F6C0A216_8C18_45E4_96DD_10D6F990DD27
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: thread_safe_queue.h 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <pthread.h>
#include <queue>
#include "zdk/mutex.h"
#include "generic/lock.h"
#include "generic/temporary.h"


template<typename T, typename Q = std::queue<T> >
class ZDK_LOCAL thread_safe_queue
{
    thread_safe_queue(const thread_safe_queue&);
    thread_safe_queue& operator=(const thread_safe_queue&);

public:
    struct queue_type : public Q
    {
        void swap(Q& other) throw()
        {
            static_cast<queue_type&>(other).c.swap(this->c);
        }
        void swap(typename Q::container_type& other) throw()
        {
            other.swap(this->c);
        }
    };

    thread_safe_queue() { }

    void push(T elem) volatile
    {
        Lock<Mutex> lock(mutex_);
        const_cast<queue_type&>(queue_).push(elem);
    }
    void pop() volatile
    {
        Lock<Mutex> lock(mutex_);
        const_cast<queue_type&>(queue_).pop();
    }
    T front() const volatile
    {
        Lock<Mutex> lock(mutex_);
        return const_cast<queue_type&>(queue_).front();
    }
    bool empty() const volatile
    {
        Lock<Mutex> lock(mutex_);
        return is_empty();
    }
    size_t size() const volatile
    {
        Lock<Mutex> lock(mutex_);
        return const_cast<queue_type&>(queue_).size();
    }
    void clear() volatile
    {
        Lock<Mutex> lock(mutex_);
        const_cast<queue_type&>(queue_) = queue_type();
    }
   /*
    template<typename U>
    void pop_all(Lock<Mutex>& lock, U& container)
    {
        assert(lock.holds(mutex_));

        typename queue_type::container_type tmp;
        this->queue_.swap(tmp);

        container.insert(container.end(), tmp.begin(), tmp.end());
        assert(this->empty());
    }
   */
    volatile Mutex& mutex() volatile { return mutex_; }

protected:
    /**
     * @note UNLOCKED
     */
    bool is_empty() const volatile
    {
        return const_cast<queue_type&>(queue_).empty();
    }

    queue_type queue_;

private:
    mutable Mutex mutex_;
};


/**
 * A thread-safe queue that signals a condition
 * when more elements are being pushed.
 */
template<typename T, typename Q = std::queue<T> >
class ZDK_LOCAL blocking_queue : public thread_safe_queue<T, Q>
{
    typedef thread_safe_queue<T, Q> base;
    typedef typename thread_safe_queue<T, Q>::queue_type queue_type;
    typedef typename Q::container_type::iterator iterator;

    mutable Condition cond_;
    bool signaled_;

    void push(T elem) volatile;

    void signal_condition()
    {
        signaled_ = true;
        cond_.broadcast();
    }
    void wait_condition(Lock<Mutex>& lock)
    {
        cond_.wait(lock);
    }

public:
    blocking_queue() : signaled_(false)
    {
    }
    void push(T elem)
    {
        Lock<Mutex> lock(this->mutex());
        const_cast<queue_type&>(this->queue_).push(elem);
        signal_condition();
    }
    void wake_up()
    {
        Lock<Mutex> lock(this->mutex());
        signal_condition();
    }
    template<typename Iter>
    void insert(iterator pos, Iter first, Iter last);

    friend class blocking_scope;

    struct blocking_scope : public Lock<Mutex>
    {
    public:
        explicit blocking_scope(volatile blocking_queue& q)
            : Lock<Mutex>(q.mutex())
        {
            while (q.is_empty())
            {
                const_cast<blocking_queue&>(q).wait_condition(*this);

                if (q.signaled_)
                {
                    q.signaled_ = false;
                    break;
                }
            }
        }

        ~blocking_scope() throw() {}

        static bool is_empty(const volatile blocking_queue& q)
        { return q.is_empty(); }
    };
};

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
#endif // THREAD_SAFE_QUEUE_H__F6C0A216_8C18_45E4_96DD_10D6F990DD27
