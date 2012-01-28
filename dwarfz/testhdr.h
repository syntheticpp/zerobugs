#ifndef TESTHDR_H__67E012B4_9292_408B_BA88_F24012B2094E
#define TESTHDR_H__67E012B4_9292_408B_BA88_F24012B2094E
//
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

namespace ABC
{
    template<typename T>
    class XYZ
    {
    public:
        explicit XYZ(T z) : z_(z) { }
        T z_;
    };

    typedef long long_type;
}
#endif // TESTHDR_H__67E012B4_9292_408B_BA88_F24012B2094E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
