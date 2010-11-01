// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

using namespace std;


void* thread_proc_one(void*)
{
    for (int i = 0; i != 2000; ++i)
    {
        cout << "thread one\n";
        usleep(10);
    }
    return 0;
}

void* thread_proc_two(void*)
{
    for (int i = 0; i != 1000; ++i)
    {
        cout << "thread two\n";
        usleep(20);
        if (i == 5)
        {
            *(int*) 0 = 1;
        }
    }
    return 0;
}

int main()
{
    pthread_t t1, t2;

    pthread_create(&t1, 0, thread_proc_one, 0);
    pthread_create(&t2, 0, thread_proc_two, 0);

    clog << "t1=" << t1 << endl;
    clog << "t2=" << t2 << endl;

    // pthread_kill(t1, SIGSTOP);
    // pthread_kill(t2, SIGSTOP);
    pthread_join(t1, 0);
    pthread_join(t2, 0);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
