#ifndef LESS_H__1CAEC4C3_D4E5_4C13_8F65_CF8146EA72A2
#define LESS_H__1CAEC4C3_D4E5_4C13_8F65_CF8146EA72A2
//
// $Id: less.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

namespace Less
{
template<typename T, typename U, typename Next>
class Pred
{
    U T::*mptr_;
    Next next_;

public:
    Pred(U T::*mptr, Next next) : mptr_(mptr), next_(next)
    { }

    bool apply(const T& lhs, const T& rhs) const
    {
        if ((lhs.*mptr_) < (rhs.*mptr_))
        {
            return true;
        }
        if ((lhs.*mptr_) > (rhs.*mptr_))
        {
            return false;
        }
        return next_.apply(lhs, rhs);
    }
    bool operator()(const T& lhs, const T& rhs) const
    { return apply(lhs, rhs); }
};
struct End
{
    template<typename T>
    static bool apply(const T& lhs, const T& rhs)
    { return false; }
};

template<typename T, typename U, typename V>
Pred<T, U, V> pred(U T::*mptr, V next)
{
    return Pred<T, U, V>(mptr, next);
}
} // namespace

#endif // LESS_H__1CAEC4C3_D4E5_4C13_8F65_CF8146EA72A2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
