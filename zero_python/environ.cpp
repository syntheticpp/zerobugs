// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: environ.cpp 714 2010-10-17 10:03:52Z root $
//
#include <string.h>
#include "environ.h"

using namespace std;


void env_to_map(const char* const* e, map<string, string>& m)
{
    for (; e && *e; ++e)
    {
        if (const char* p = strchr(*e, '='))
        {
            m[string(*e, p)] = string(p + 1);
        }
    }
}


void map_to_env(const map<string, string>& m, SArray& e)
{
    for (map<string, string>::const_iterator i = m.begin();
        i != m.end();
        ++i)
    {
        e.push_back(i->first + "=" + i->second);
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
