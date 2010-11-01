//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: test_task_pool.cpp 710 2010-10-16 07:09:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <iostream>
#include "../task_pool.h"

using namespace std;


class TestTask : public Task
{
    bool done_;

    void run()
    {
        done_ = true;
        usleep(10);

        clog << this << " thread_id=" << pthread_self() << ": done" << endl;
    }

public:
    TestTask() : done_(false) { }

    ~TestTask() throw() { }

    bool is_done() const { return done_; }
};


void no_delete(TestTask*) { }

void run_pool(TestTask* task, size_t nTasks, size_t nThreads)
{
    TaskPool pool(nThreads);

    for (size_t i = 0; i != nTasks; ++i)
    {
        assert(!task[i].is_done());

        pool.schedule(TaskPool::TaskPtr(&task[i], no_delete));
    }

    pool.wait_until_empty();

}


int main()
{
    TestTask tasks [11];

    run_pool(tasks, sizeof(tasks) / sizeof(tasks[0]), 4);
    return 0;
}
