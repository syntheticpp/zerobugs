#ifndef TASK_POOL_H__329140B1_CC53_4BF6_8DEE_87A90F9E4D10
#define TASK_POOL_H__329140B1_CC53_4BF6_8DEE_87A90F9E4D10
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
#include <deque>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "zdk/mutex.h"


////////////////////////////////////////////////////////////////
class ZDK_LOCAL Task
{
    virtual void setup() { }

    virtual void run() = 0;

    virtual void done() { }

public:
    virtual ~Task() throw() { }

    /**
     * @note template method pattern
     */
    void execute()
    {
        setup();
        run();
        done();
    }
};



////////////////////////////////////////////////////////////////
class ZDK_LOCAL TaskPool
{
public:
    typedef boost::shared_ptr<Task> TaskPtr;
    typedef std::deque<TaskPtr> Queue;

private:
    /**
     * Extract a task from the queue, block if empty.
     * @return false if cancelled, true otherwise.
     */
    bool get_task(TaskPtr&);

    /**
     * create nThreads worker threads
     */
    void init(size_t nThreads);

    static void* execute_from_queue(void*);

public:
    TaskPool();

    explicit TaskPool(size_t nThreads);

    ~TaskPool() throw();

    void cancel() throw();

    void schedule(const TaskPtr&);

    void wait_until_empty();

private:
    Queue       queue_;
    Mutex       mutex_;
    Condition   queueEmpty_;
    Condition   queueNotEmpty_;
    bool        cancelled_;

    std::vector<pthread_t> threads_;
};


#endif // TASK_POOL_H__329140B1_CC53_4BF6_8DEE_87A90F9E4D10
