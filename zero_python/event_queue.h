#ifndef EVENT_QUEUE_H__51D45F3B_A2B6_407E_8444_954750A7C079
#define EVENT_QUEUE_H__51D45F3B_A2B6_407E_8444_954750A7C079
//
// $Id: event_queue.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include "thread_safe_queue.h"


/**
 * A queue which raises an event when elements
 * are pushed.
 *
 * @note methods called from the main thread are volatile
 * to prevent accidentally calling them from other threads.
 */
template<typename T>
class event_queue : public thread_safe_queue<T>
{
    typedef thread_safe_queue<T> base_type;

    void push_(T elem) volatile
    {
        Lock<Mutex> lock(this->mutex());
        event_queue* self = const_cast<event_queue*>(this);

        bool newEventBurst = self->queue_.empty();
        self->queue_.push(elem);
        raise_event(newEventBurst);
    }

protected:
    virtual void raise_event(bool) volatile { }

public:
    event_queue() { }

    void push(T elem) volatile { this->push_(elem); }
    void push(T elem) { push_(elem); }
};

#endif // EVENT_QUEUE_H__51D45F3B_A2B6_407E_8444_954750A7C079
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
