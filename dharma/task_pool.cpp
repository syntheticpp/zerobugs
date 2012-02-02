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

#include "zdk/config.h"
#include "zdk/log.h"
#include "generic/lock.h"
#include "system_error.h"
#include "task_pool.h"
#include <iostream>


TaskPool::TaskPool() : cancelled_(false)
{
#ifdef __linux__
    size_t nProcs = get_nprocs();
    if (nProcs > 1)
    {
        --nProcs;
    }
    init(nProcs);
#else
    init(1);
#endif
}


TaskPool::TaskPool(size_t nThreads) : cancelled_(false)
{
    init(nThreads);
}


TaskPool::~TaskPool() throw()
{
    cancel();

    for (std::vector<pthread_t>::iterator i = threads_.begin();
         i != threads_.end();
         ++i
        )
    {
        pthread_join(*i, NULL);
    }
}


void TaskPool::cancel() throw()
{
    Lock<Mutex> lock(mutex_);
    cancelled_ = true;
    queueNotEmpty_.broadcast(std::nothrow);
}


bool TaskPool::get_task(TaskPtr& task)
{
    Lock<Mutex> lock(mutex_);

    while (!cancelled_ && queue_.empty())
    {
        queueNotEmpty_.wait(lock);
    }
    if (cancelled_)
    {
        return false;
    }
    task = queue_.front();
    queue_.pop_front();
    queueEmpty_.broadcast();
    return true;
}


void TaskPool::schedule(const TaskPtr& task)
{
    Lock<Mutex> lock(mutex_);

    queue_.push_back(task);
    queueNotEmpty_.broadcast();
}


void* TaskPool::execute_from_queue(void* p)
{
    assert(p);

    TaskPool* self = reinterpret_cast<TaskPool*>(p);

    try
    {
        TaskPtr task;

        while (self->get_task(task))
        {
            task->execute();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << __func__ << ": " << e.what() << std::endl;
		dbgout(-1) << e.what() << std::endl;	// log it
    }
    return NULL;
}


void TaskPool::init(size_t nThreads)
{
    threads_.reserve(nThreads);

    for (size_t i = 0; i != nThreads; ++i)
    {
        pthread_t thread = 0;
        int err = pthread_create(&thread, 0, execute_from_queue, this);

        if (err)
        {
            throw SystemError("TaskPool::init", err);
        }
        threads_.push_back(thread);
    }
}


void TaskPool::wait_until_empty()
{
    Lock<Mutex> lock(mutex_);
    while (!queue_.empty())
    {
        queueEmpty_.wait(lock);
    }
}

