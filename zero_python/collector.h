#ifndef COLLECTOR_H__9DAA50B9_4129_41EA_8585_5B265FE238E2
#define COLLECTOR_H__9DAA50B9_4129_41EA_8585_5B265FE238E2
//
// $Id: collector.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/python.hpp>
#include "generic/lock.h"
#include "zdk/enum.h"
#include "zdk/mutex.h"


template<typename R>
struct ResultBase
{
    R default_result() { return R(); }
};
template<> struct ResultBase<bool>
{
    bool default_result() { return true; }
};

/**
 * Collects the items sent to notify() into a python::list
 */
template<typename T,
         typename R = void,
         typename B = EnumCallback<T*, R>,
         typename C = boost::python::list>
class Collector : public B
                , protected ResultBase<R>
{
    mutable Mutex mutex_;
    C collection_;

    R notify(T* elem)
    {
        Lock<Mutex> lock(mutex_);
        collection_.append(RefPtr<T>(elem));

        return this->default_result();
    }

public:
   /**
    * assumption: the collection only holds a few elements,
    * so returning by value shouldn't be too bad.
    */
    C get() const
    {
        Lock<Mutex> lock(mutex_);
        return collection_;
    }
};


#endif // COLLECTOR_H__9DAA50B9_4129_41EA_8585_5B265FE238E2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
