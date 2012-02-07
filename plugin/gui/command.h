#ifndef COMMAND_H__9267DAA0_D37B_4FDA_9C24_608B71C90724
#define COMMAND_H__9267DAA0_D37B_4FDA_9C24_608B71C90724
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

#include <typeinfo>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include "zdk/ref_ptr.h"
#include "zdk/zobject_impl.h"
#include "piped_queue.h"

/**
 * Base class for requests exchanged between the main
 * debugger thread and the UI thread -- an instance of
 * the Command pattern.
 *
 * @note There are functions that only work when called
 * from the main debugger thread, the one that has attached
 * to the debugee with the pthread() call. Similarly, UI
 * functions should be invoked from the gtk event loop,
 * at well-defined times (from a callback, event handler,
 * etc). This is why we need a mechanism for "marshalling"
 * calls from one thread to another.
 * To post requests to the UI, we use the gtk main input
 * signal/slot that notifies the event loop that some file
 * descriptor has data available for read(). The main thread
 * pushes an InterThreadCommand object to a request queue,
 * and then writes one dummy byte to a pipe (just to notify
 * the UI). The UI handler for the input event pops the
 * command from the queue and executes it (and optionally
 * pushes a "response" InterThreadCommand object to a
 * response queue, for the main thread to pick up).
 */
DECLARE_ZDK_INTERFACE_(InterThreadCommand, RefCounted)
{
    virtual ~InterThreadCommand() { }

    virtual bool execute() = 0;

    virtual const char* name() const = 0;

    virtual bool is_equal(const InterThreadCommand*) const
    { return false; }

    /**
     * @return true if safe to execute the command
     * without any acquiring a mutex
     */
    virtual bool is_safe() const { return false; }
};


typedef RefPtr<InterThreadCommand> CommandPtr;

typedef piped_queue<CommandPtr> RequestQueue;
typedef blocking_queue<CommandPtr> ResponseQueue;


/**
 * Adapts a functor into an InterThreadCommand, so that
 * we can push the functors in a container,
 * and call them polymorphically.
 */
template<typename F, bool R = false>
class ZDK_LOCAL CommandAdapter : public ZObjectImpl<InterThreadCommand>
{
public:
    explicit CommandAdapter(const F& f, bool safe = false)
        : f_(f), safe_(safe) { }

    ~CommandAdapter() throw() { }

private:
    bool execute() { f_(); return R; }

    const char* name() const { return typeid(F).name(); }

    bool is_safe() const { return safe_; }

private:
    F f_;
    bool safe_;
};



template<typename F>
class ZDK_LOCAL Wrapper : public ZObjectImpl<InterThreadCommand>
{
public:
    explicit Wrapper(const F& f) : f_(f) { }

    ~Wrapper() throw() { }

private:
    bool execute() { return f_(); }

    const char* name() const { return typeid(F).name(); }

    virtual bool is_safe() const { return true; }

private:
    F f_;
};


class ZDK_LOCAL ResumeCommand : public ZObjectImpl<InterThreadCommand>
{
    bool execute() { return true; }
    const char* name() const { return "resume"; }
};


//
// Convenience template functions that help the compiler
// automatically figure the types of the template params.
//
template<typename T>
inline CommandPtr ZDK_LOCAL safe_command(T f)
{
    return CommandPtr(new CommandAdapter<T>(f, true));
}

template<typename W, typename U>
inline void ZDK_LOCAL post_safe_command(void (W::*fun)(U), W* wnd, U arg)
{
    assert(wnd);
    wnd->post_request(safe_command(boost::bind(fun, wnd, arg)));
}

template<typename T>
inline CommandPtr ZDK_LOCAL command(const T& f)
{
    return CommandPtr(new CommandAdapter<T>(f));
}

template<typename T>
inline CommandPtr ZDK_LOCAL resume_cmd(const T& f)
{
    return CommandPtr(new CommandAdapter<T, true>(f));
}

template<typename T>
inline CommandPtr ZDK_LOCAL wrap(const T& f)
{
    return CommandPtr(new Wrapper<T>(f));
}

template<typename R, typename T>
inline CommandPtr ZDK_LOCAL command(R (T::*fun)(), T* obj)
{
    return command(boost::bind(fun, obj));
}

template<typename R, typename T>
inline CommandPtr ZDK_LOCAL resume_cmd(R (T::*fun)(), T* obj)
{
    return resume_cmd(boost::bind(fun, obj));
}

template<typename R, typename T, typename U>
inline CommandPtr ZDK_LOCAL command(R (T::*fun)(U), T* obj, U arg)
{
    return command(boost::bind(fun, obj, arg));
}

template<typename R, typename T, typename U>
inline CommandPtr ZDK_LOCAL command(R (T::*fun)(U) const, T* obj, U arg)
{
    return command(boost::bind(fun, obj, arg));
}

template<typename R, typename T, typename U>
inline CommandPtr ZDK_LOCAL command(R (T::*fun)(U) volatile, T* obj, U arg)
{
    //return command(boost::bind(boost::mem_fn(fun), obj, arg));
    return command(boost::bind((R (T::*)(U))fun, obj, arg));
}

template<typename R, typename T, typename U>
inline CommandPtr ZDK_LOCAL command(R (T::*fun)(U), volatile T* obj, U arg)
{
    return command(boost::bind(fun, const_cast<T*>(obj), arg));
}

template<typename T, typename U, typename R>
inline CommandPtr ZDK_LOCAL resume_cmd(R (T::*fun)(U), T* obj, U arg)
{
    return resume_cmd(boost::bind(fun, obj, arg));
}

template<typename T, typename U, typename V, typename W, typename R>
inline CommandPtr ZDK_LOCAL resume_cmd(
    R (T::*fun)(U, V, W), T* obj, U arg1, V arg2, W arg3)
{
    return resume_cmd(boost::bind(fun, obj, arg1, arg2, arg3));
}

template<typename R, typename T, typename U, typename V>
inline CommandPtr ZDK_LOCAL command(R (T::*fun)(U, V), T* obj, U arg1, V arg2)
{
    return command(boost::bind(fun, obj, arg1, arg2));
}

template<typename R, typename T, typename A1, typename A2, typename A3>
inline CommandPtr ZDK_LOCAL
command(R (T::*fun)(A1, A2, A3), T* obj, A1 arg1, A2 arg2, A3 arg3)
{
    return command(boost::bind(fun, obj, arg1, arg2, arg3));
}



template<typename R, typename T, typename A1, typename A2, typename A3, typename A4>
inline CommandPtr ZDK_LOCAL
command(R (T::*fun)(A1, A2, A3, A4), T* obj, A1 arg1, A2 arg2, A3 arg3, A4 arg4)
{
    return command(boost::bind(fun, obj, arg1, arg2, arg3, arg4));
}


template<typename R,
         typename T,
         typename A1,
         typename A2,
         typename A3,
         typename A4,
         typename A5>
inline CommandPtr ZDK_LOCAL command(R (T::*fun)(A1, A2, A3, A4, A5),
                   T* obj,
                   A1 arg1,
                   A2 arg2,
                   A3 arg3,
                   A4 arg4,
                   A5 arg5)
{
    return command(boost::bind(fun, obj, arg1, arg2, arg3, arg4, arg5));
}


/*
template<typename U, typename V>
inline CommandPtr ZDK_LOCAL function(void (*fun)(U, V), U arg1, V arg2)
{
    return command(boost::bind(fun, arg1, arg2));
} */


template<typename W>
inline void ZDK_LOCAL post_command(void (W::*fun)(), W* wnd)
{
    assert(wnd);
    wnd->post_request(command(boost::bind(fun, wnd)));
}

template<typename W>
inline void ZDK_LOCAL post_command(void (W::*fun)(), volatile W* wnd)
{
    assert(wnd);
    wnd->post_request(command(boost::bind(fun, const_cast<W*>(wnd))));
}

template<typename W, typename U>
inline void ZDK_LOCAL post_command(void (W::*fun)(U), W* wnd, U arg)
{
    assert(wnd);
    wnd->post_request(command(boost::bind(fun, wnd, arg)));
}

template<typename W, typename U>
inline void ZDK_LOCAL post_command(void (W::*fun)(U), volatile W* wnd, U arg)
{
    assert(wnd);
    wnd->post_request(
        command(boost::bind(fun, const_cast<W*>(wnd), arg)));
}

template<typename W, typename U>
inline void ZDK_LOCAL post_command(void (W::*fun)(U) volatile, volatile W* wnd, U arg)
{
    assert(wnd);
    wnd->post_request(
        command(boost::bind(fun, const_cast<W*>(wnd), arg)));
}


template<typename W, typename U>
inline void ZDK_LOCAL post_command
 (
    void (W::*fun)(U),
    boost::shared_ptr<W> wnd,
    U arg
 )
{
    post_command(fun, wnd.get(), arg);
}

template<typename W, typename U>
inline void ZDK_LOCAL post_request
 (
    void (W::*fun)(U),
    boost::shared_ptr<W> wnd,
    U arg
 )
{
    post_command(fun, wnd.get(), arg);
}

template<typename W, typename U, typename V>
inline void ZDK_LOCAL post_command(void (W::*fun)(U, V), W* wnd, U arg1, V arg2)
{
    assert(wnd);
    wnd->post_request(command(boost::bind(fun, wnd, arg1, arg2)));
}


template<typename W, typename U, typename V, typename T>
inline void ZDK_LOCAL post_command(void (W::*fun)(U, V, T), W* wnd, U arg1, V arg2, T arg3)
{
    assert(wnd);
    wnd->post_request(command(boost::bind(fun, wnd, arg1, arg2, arg3)));
}


#endif // COMMAND_H__9267DAA0_D37B_4FDA_9C24_608B71C90724
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
