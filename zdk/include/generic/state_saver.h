#ifndef STATE_SAVER_H__1059590469
#define STATE_SAVER_H__1059590469
//
// $Id: state_saver.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iosfwd>
#include "generic/export.h"


template<typename T, typename S>
struct StateTraits;


template<typename T,
         typename S,
         typename Traits=StateTraits<T, S> >
class ZDK_LOCAL StateSaver
{
public:
    explicit StateSaver(T& ref)
        : ref_(ref), state_(Traits::get(ref))
     {
     }

     ~StateSaver() throw()
     {
        Traits::set(ref_, state_);
     }

private:
    T& ref_;
    S state_;
};


template<>
struct ZDK_LOCAL StateTraits<std::ios, std::ios::fmtflags>
{
    // static bool get(const std::ios& ios)
    static std::ios::fmtflags get(const std::ios& ios)
    {
        return ios.flags();
    }

    static void set(std::ios& ios, std::ios::fmtflags f)
    {
        ios.flags(f);
    }
};

#endif // STATE_SAVER_H__1059590469
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
