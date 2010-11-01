#ifndef PIPED_QUEUE_H__6EB2FC9E_50C1_4EB0_B725_6D277A5EAF97
#define PIPED_QUEUE_H__6EB2FC9E_50C1_4EB0_B725_6D277A5EAF97
//
// $Id: piped_queue.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <iostream>
#include "thread_safe_queue.h"
#include "dharma/pipe.h"


/**
 * An inter-thread queue. The reader thread listens on
 * the ouput end of the pipe (with select, or by registering
 * an IO event notification handler, a mechanism provided by
 * X and Gtk). Upon a "data-available" event, the reader drains
 * the queue (by calling pop_all).
 */
template<typename T>
class piped_queue : public thread_safe_queue<T>
{
    typedef thread_safe_queue<T> base_type;
    typedef typename base_type::queue_type::container_type container_type;

    Pipe pipe_;

    void push_(T elem) volatile
    {
        Lock<Mutex> lock(this->mutex());

        if (const_cast<piped_queue*>(this)->queue_.empty())
        {
            char dummy = 0;
            while (!pipe_.write(dummy, std::nothrow))
            {
                std::cerr << __func__ << ": error writing pipe\n";
            }
        #ifdef DEBUG_PIPED_QUEUE
            time_t now = time(0);
            std::clog << __func__ << ": " << ctime(&now);
        #endif
        }

        const_cast<piped_queue*>(this)->queue_.push(elem);
    }

public:
    piped_queue() { }

    Pipe& pipe() { return pipe_; }

    void push(T elem) volatile { this->push_(elem); }
    void push(T elem) { push_(elem); }

    void pop_all(container_type& container)
    {
        Lock<Mutex> lock(this->mutex());

        container_type tmp;
        this->queue_.swap(tmp);

        container.insert(container.end(), tmp.begin(), tmp.end());

        char dummy = 0;
        if (!pipe_.read(dummy, std::nothrow))
        {
            std::cerr << __func__ << ": error reading pipe\n";
        }
    #ifdef DEBUG_PIPED_QUEUE
        time_t now = time(0);
        std::clog << __func__ << ": " << ctime(&now);
    #endif
        assert(this->queue_.empty());
    }
};

#endif // PIPED_QUEUE_H__6EB2FC9E_50C1_4EB0_B725_6D277A5EAF97
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
