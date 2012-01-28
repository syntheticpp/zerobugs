// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <assert.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "dharma/environ.h"
#include "dharma/sarray.h"
#include "dharma/system_error.h"


using namespace std;


bool env::get_bool(const char* name, bool defaultValue)
{
    assert(name);
    bool flag = defaultValue;

    if (const char* const p = getenv(name))
    {
        istringstream str(p);
        if (!isdigit(*p))
        {
            str.setf(ios::boolalpha);
        }
        str >> flag;
    }
    return flag;
}



string env::get_string(const char* name, const char* defaultStr)
{
    assert(name);
    string str;
    if (const char* p = getenv(name))
    {
        str.assign(p);
    }
    else if (defaultStr)
    {
        str.assign(defaultStr);
    }

    return str;
}



int env::get(const char* name, int defaultVal)
{
    assert(name);
    int result = defaultVal;

    if (const char* p = getenv(name))
    {
        result = strtol(p, 0, 0);
    }

    return result;
}



void env::read(istream& f, SArray& env)
{
    string str;
    char c = 0;
    bool hasEquals = false;

    while (f >> noskipws >> c)
    {
        if (c)
        {
            if (c == '=' && !str.empty())
            {
                hasEquals = true;
            }
            str += c;
        }
        else
        {
            if (hasEquals)
            {
                env.push_back(str);
            }
            hasEquals = false; // reset
            str.clear();
        }
    }
    if (!str.empty() && hasEquals)
    {
        env.push_back(str);
    }
}



void env::read(const string& filename, SArray& env)
{
    ifstream f(filename.c_str());

    read(f, env);
}



void env::write(const string& filename, const char* const* env)
{
    ofstream f(filename.c_str());
    if (!f)
    {
        throw SystemError(filename);
    }
    f.exceptions(ios::eofbit | ios::failbit | ios::badbit);

    while (env && *env)
    {
        f << *env;
        f << (char)0;

        ++env;
    }
    //f << (char)0;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
