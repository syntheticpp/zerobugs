#ifndef OBSERVER_IMPL_H__09D783FD_69AF_43A6_863A_05A9E1D51714
#define OBSERVER_IMPL_H__09D783FD_69AF_43A6_863A_05A9E1D51714
//
// $Id: observer_impl.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <algorithm>
#include <set>
#include "generic/lock.h"
#include "zdk/observer.h"
#include "zdk/zobject_impl.h"
#include "zdk/mutex.h"
#include "zdk/weak_ptr.h"


/**
 * Template implementation of the Observer interface
 */
template<typename T = ZObjectImpl<Observer> >
class ZDK_LOCAL ObserverImpl : public T
{
public:
    virtual ~ObserverImpl() throw() { }

    // compiler-generated ctor is fine
    // ObserverImpl() {}

BEGIN_INTERFACE_MAP(ObserverImpl)
    INTERFACE_ENTRY_INHERIT(T)
END_INTERFACE_MAP()

protected:
    void on_state_change(Subject*) { }
};



/**
 * Template implementation of the Subject interface
 */
template<typename T = Subject>
class ZDK_LOCAL SubjectImpl : public ZObjectImpl<T>
{
    typedef std::set<WeakPtr<Observer> > ObserverSet;

public:
    SubjectImpl() {}

    virtual ~SubjectImpl() throw()
    {
    }

BEGIN_INTERFACE_MAP(SubjectImpl<T>)
    INTERFACE_ENTRY(T)
END_INTERFACE_MAP()

    bool attach(Observer* obs)
    {
        assert(obs);
        assert(obs->ref_count());

        Lock<Mutex> lock(mx_);
        ObserverSet::value_type ptr(obs);
        if (observers_.find(ptr) == observers_.end())
        {
            observers_.insert(ptr);
            return true;
        }
        return false;
    }

    bool detach(Observer* obs)
    {
        Lock<Mutex> lock(mx_);
        ObserverSet::iterator i = observers_.find(obs);
        if (i != observers_.end())
        {
            observers_.erase(i);
            return true;
        }
        return false;
    }

    void detach()
    {
        Lock<Mutex> lock(mx_);
        observers_.clear();
    }

    void notify_state_change()
    {
        ObserverSet tmp;
        {
            Lock<Mutex> lock(mx_);
            tmp = observers_;
        }
        ObserverSet::iterator i = tmp.begin();
        for (; i != tmp.end(); ++i)
        {
            if (RefPtr<Observer> obs = i->ref_ptr())
            {
                obs->on_state_change(this);
            }
        }
    }

private:
    // non-copyable, non-assignable for simplicity
    SubjectImpl(const SubjectImpl&);
    SubjectImpl& operator=(const SubjectImpl&);

    ObserverSet observers_;
    Mutex mx_;
};


/**
 * Observer implementation that delegates on_state_change
 * notifications to other objects.
 */

template<typename T>
class ZDK_LOCAL ObserverDelegate : public ObserverImpl<>
{
    T* p_;

    void on_state_change(Subject* sub) { p_->on_state_change(sub); }

    ObserverDelegate(T* p) : p_(p) { }

public:
    static RefPtr<Observer> create(T* p)
    { return new ObserverDelegate(p); }
};


template<typename T>
inline RefPtr<Observer> ZDK_LOCAL create_observer_delegate(T* p)
{
    return ObserverDelegate<T>::create(p);
}

#endif // OBSERVER_IMPL_H__09D783FD_69AF_43A6_863A_05A9E1D51714

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
