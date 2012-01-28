#ifndef ASYNC_FUN_H__A6E2AA4C_7786_4A74_AA68_23978353EC0E
#define ASYNC_FUN_H__A6E2AA4C_7786_4A74_AA68_23978353EC0E
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
#include "zdk/export.h"
#include "zdk/stdexcept.h"
#include "dharma/task_pool.h"
#include <assert.h>
#include <iostream>


template<typename T>
class ZDK_LOCAL AsyncFun : public Task
{
    typedef void (T::*Fun)() const;

    T*  obj_;
    Fun fun_;
    bool done_;

    mutable Condition completed_;
    mutable Mutex mutex_;

    void run()
    {
        Lock<Mutex> lock(mutex_);
        if (!done_)
        {
            try
            {
                (obj_->*fun_)();
            }
            catch (const std::exception& e)
            {
                std::cerr << __func__ << ": " << e.what() << std::endl;
            }
        }
    }

public:
    AsyncFun(T* obj, Fun fun)
        : obj_(obj)
        , fun_(fun)
        , done_(false)
    { }

    ~AsyncFun() throw () { }

    void wait_for_completion() const
    {
        Lock<Mutex> lock(mutex_);
        while (!done_)
        {
            completed_.wait(lock);
        }
    }

    void done()
    {
        Lock<Mutex> lock(mutex_);
        done_ = true;

        completed_.broadcast();
    }
};


#endif // ASYNC_FUN_H__A6E2AA4C_7786_4A74_AA68_23978353EC0E
