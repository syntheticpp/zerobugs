#ifndef MESSAGE_H__543C7419_09E6_4504_9C76_4DB5D8A8E33F
#define MESSAGE_H__543C7419_09E6_4504_9C76_4DB5D8A8E33F
//
// $Id: message.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "generic/lock.h"
#include "zdk/mutex.h"
#include "zdk/zobject.h"


DECLARE_ZDK_INTERFACE_(Message, ZObject)
{
    DECLARE_UUID("abc03d1f-7c88-4e37-9a3b-3d0480a92eaf")

    virtual const char* name() const = 0;

    virtual bool dispatch() = 0;

    virtual bool result() const = 0;
};


template<typename R, typename F>
struct MessageTraits
{
    static R call(const F& functor) { return functor(); }
};

template<typename F>
struct MessageTraits<void, F>
{
    static bool call(F& functor)
    {
        functor();
        return false;
    }
};


/**
 * Adapts a functor into a Message
 */
template<typename F>
class ZDK_LOCAL MessageAdapter : public ZObjectImpl<Message>
{

public:
    typedef typename F::result_type result_type;

    MessageAdapter(const F& fun, const char* name)
        : fun_(fun), result_(false)
    {
        if (name)
        {
            name_.assign(name);
        }
    }

    virtual ~MessageAdapter() throw() { }

    bool dispatch()
    {
        result_ = MessageTraits<result_type, F>::call(fun_);
        return result_;
    }

    const char* name() const
    {
        return name_.empty() ? typeid(F).name() : name_.c_str();
    }

    bool result() const { return result_; }

private:
    F fun_;
    std::string name_;
    bool result_;
};



/**
 * Synchronous command adapter, used by Marshaller::send_command
 */
template<typename F>
class ZDK_LOCAL SyncMessageAdapter : public MessageAdapter<F>
{
    Condition cond_;
    bool completed_, result_;
    Mutex mutex_;

    class AutoBroadcast : boost::noncopyable
    {
        Condition& cond_;

    public:
        explicit AutoBroadcast(Condition& cond) : cond_(cond) { }
        ~AutoBroadcast() { cond_.broadcast(); }
    };

public:
    SyncMessageAdapter(const F& fun, const char* name)
        : MessageAdapter<F>(fun, name)
        , completed_(false)
        , result_(false)
    { }

    ~SyncMessageAdapter() throw() { }

    bool dispatch()
    {
        AutoBroadcast autoBroadcast(cond_);
        result_ = MessageAdapter<F>::dispatch();

        Lock<Mutex> lock(mutex_);
        completed_ = true;
        return result_;
    }

    void wait()
    {
        Lock<Mutex> lock(mutex_);
        while (!completed_)
        {
            cond_.wait(lock);
        }
        completed_ = false;
    }

    bool result() const { return result_; }
};

#endif // MESSAGE_H__543C7419_09E6_4504_9C76_4DB5D8A8E33F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
