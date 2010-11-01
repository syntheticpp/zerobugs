#ifndef EXPECT_H__1149FECA_9C51_4377_9786_1F1F6E3DAF81
#define EXPECT_H__1149FECA_9C51_4377_9786_1F1F6E3DAF81
//
// $Id: expect.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "expect.cpp"


#define STRINGIFY(x) #x

#define EXPECT(val, t) \
    while(!expect(val, (t), STRINGIFY(val), __FILE__, __LINE__)) \
        return 0;

#define EXPECT_EITHER(v1, v2, t) \
    while(!expect(v1, v2, (t), STRINGIFY(v1 or v2))) \
        return 0;


#endif // EXPECT_H__1149FECA_9C51_4377_9786_1F1F6E3DAF81
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
