//
// $Id: expect.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Utilities used in stab parsing.
//
#include <cctype>
#include <iostream>


inline bool expect(
    int expected,
    int actual,
    const char* name,
    const char* file,
    unsigned int line)
{
    bool result = (expected == actual);

    if (!result)
    {
        // std::cerr << "STABZ: expecting " << name;
        std::cerr << "STABZ: " << file << ':' << line;
        std::cerr << " expecting " << name;
        std::cerr << " (" << expected;
        std::cerr << "), got " << actual;

        if (isprint(actual))
        {
            std::cerr << " `"<< static_cast<char>(actual) << "'";
        }
        std::cerr << std::endl;
        assert(false);
    }

    return result;
}


inline bool expect(int expect1, int expect2, int actual, const char* name)
{
    if (expect1 != actual && expect2 != actual)
    {
        std::cerr << "STABZ: expecting " << name;
        std::cerr << " (" << expect1 << " or " << expect2;
        std::cerr << "), got " << actual;

        if (isprint(actual))
        {
            std::cerr << " `"<< static_cast<char>(actual) << "'";
        }
        std::cerr << std::endl;

        assert(false);
        return false;
    }

    return true;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
