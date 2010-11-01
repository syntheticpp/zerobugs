#ifndef UTIL_H__1BBB616B_71EA_4EB1_B8BC_AD2DCF6C43A1
#define UTIL_H__1BBB616B_71EA_4EB1_B8BC_AD2DCF6C43A1
//
// $Id: util.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

namespace Stab
{
    /**
     * Given a stab type, return it's name, as a string.
     * This is different from the stab string itself.
     */
    extern "C" const char* get_name(int stab_type);


    /**
     * Concatenate the prefix and the filename into a
     * full path, if necessary, and return a malloc-ed
     * string. Assumes ASCII encoding.
     */
    extern "C" char* fullpath(const char* prefix, const char* path);
}

#endif // UTIL_H__1BBB616B_71EA_4EB1_B8BC_AD2DCF6C43A1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
