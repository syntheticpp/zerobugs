#ifndef COPY_IF_H__1060470181
#define COPY_IF_H__1060470181
//
// $Id: copy_if.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Thanks to Scott Meyers (Effective STL, Item 36)
//
#include "generic/export.h"


template<typename InputIter, typename OutIter, typename Pred>
inline OutIter DSO_LOCAL
copy_if(InputIter first, InputIter last, OutIter outIter, Pred p)
{
    for (; first != last; ++first)
    {
        if (p(*first))
        {
            *outIter++ = *first;
        }
    }
    return outIter;
}

#endif // COPY_IF_H__1060470181
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
