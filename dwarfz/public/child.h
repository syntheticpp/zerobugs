#ifndef CHILD_OF_H__B7B4BE4B_EC0B_45D8_9A8F_033BE936C1A0
#define CHILD_OF_H__B7B4BE4B_EC0B_45D8_9A8F_033BE936C1A0
//
// $Id: child.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

namespace Dwarf
{
    template<typename T> struct Child
    {
        typedef T parent_type;
    };
}

#endif // CHILD_OF_H__B7B4BE4B_EC0B_45D8_9A8F_033BE936C1A0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
