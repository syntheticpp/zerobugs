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
//
#include <algorithm>
#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include "unmangle.h"

extern "C"
{
 //#include "demangle.h" // use cplus_demangle for comparison
}

#define CXXFILT
// compatible with cplus_demangle(), for comparison and testing
#define CPLUS_DEMANGLE_COMPAT
#define CPLUS_DEMANGLE_LEGACY_COMPAT
#define CPLUS_DEMANGLE_BUG_COMPAT
//#include "decoder.h"
//#include "unmangle.cpp"

using namespace std;

static int flags = 0; // demangle flags
static bool echo = false;
static bool gnu = false;
static bool silent = false;
static bool batch = false;
static bool diff = false;
static bool count_cplus_failures = false;
static bool verbose = false;
static bool progress = false;

const static int CPLUS_FLAGS = DMGL_AUTO | DMGL_PARAMS | DMGL_ANSI;

// Ian Lance Taylor's V3 demangler, for comparison and testing
extern "C" char *cxa_demangle PARAMS ((const char *, char *, size_t *, int *));

// dummy
extern "C" char* demangle_d(const char*)
{
    return NULL;
}


static void diff_algo(const string& name, size_t& count)
{
#if 0
    char* str1 = cplus_demangle(name.c_str(), CPLUS_FLAGS);
    char* str2 = unmangle(name.c_str(), 0, 0, flags);
    char* str3 = 0;
    //if ((str1 != 0) != (str2 != 0) || (str1 && strcmp(str1, str2)))
    if (!str_equal(str1, str2))
    {
        if (str1 || count_cplus_failures)
        {
            //int status = 0;
            //str3 = cxa_demangle(name.c_str(), 0, 0, &status);
            //if (str3 && strcmp(str2, str3) != 0)
            {
                ++count;
                if (!silent)
                {
                    cout << "---\n";
                    cout << name << endl;
                    cout << (str1 ? str1 : "(null)") << endl;
                    cout << "***\n";
                    cout << (str2 ? str2 : "(null)") << endl;
                /*
                    cout << "__cxa_demangle votes: ";
                    if (str2 && str3 && strcmp(str3, str2) == 0)
                    {
                        cout << "UNMANGLE";
                    }
                    else if (str1 && str3 && strcmp(str3, str1) == 0)
                    {
                        cout << "cplus_demangle";
                    }
                    else cout << "neutral";
                    cout << endl; */
                }
            }
        }
    }
    free(str1);
    free(str2);
    free(str3);
#endif
}

static void inline demangle(const string& name, size_t& count)
{
    int status = 0;
#if 0
    char* str = gnu ? cplus_demangle(name.c_str(), CPLUS_FLAGS)
                    //cxa_demangle(name.c_str(), 0, 0, &status)
                    : unmangle(name.c_str(), 0, &status, flags);
#else
    char* str = unmangle(name.c_str(), 0, &status, flags);
#endif
    if (str)
    {
        ++count;
        if (!silent)
        {
            cout << str << endl;
        }
        free(str);
    }
    else if (!silent)
    {
        cout << name << endl;
    }
}

int main(int argc, char **argv)
{
    try
    {
        size_t count = 0;
        istream* in = &cin;
        auto_ptr<ifstream> ifs;

        for (--argc, ++argv; argc; --argc, ++argv)
        {
            if (**argv == '-')
            {
                for (size_t i = 1; (*argv)[i]; ++i)
                {
                    switch ((*argv)[i])
                    {
                    case 'b': batch = true; break;
                    case 'd': diff = true; gnu = false; break;
                    case 'e': echo = true; break;
                    case 'g': if (!diff) gnu = true; break;
                    case 'n': flags |= UNMANGLE_NOFUNARGS; break;
                    case 'p': progress = true; break;
                    case 's': silent = true; break;
                    case 'k': count_cplus_failures = true; break;
                    case 'v': verbose = true; break;
                    default: cerr << "invalid option\n"; return -1;
                    }
                }
            }
            else
            {
                ifs.reset(new ifstream(*argv));
                in = ifs.get();
                if (!*in)
                {
                    cerr << "could not open file: " << *argv << endl;
                    return -1;
                }
            }
        }
        vector<string> symbols;
        time_t timeStart(time(0));
        INSTRUMENTED_("cxxfilt");
        while (!in->eof())
        {
            string name;
            (*in) >> name;
            if (name.empty())
            {
                break;
            }
            if (echo)
            {
                cout << name << endl;
            }
            if (batch)
            {
                symbols.push_back(name);
            }
            else if (diff)
            {
                diff_algo(name, count);
            }
            else
            {
                demangle(name, count);
            }
        }
        if (batch)
        {
            INSTRUMENTED_("batch");
            size_t n = 0;
            for (vector<string>::const_iterator i = symbols.begin();
                i != symbols.end();
                ++i)
            {
                if (diff)
                {
                    diff_algo(*i, count);
                }
                else
                {
                    demangle(*i, count);
                    if (progress)
                    {
                        cout << dec << ++n << " / " << symbols.size() << "\r";
                    }
                }
            }
        }
        cout << (diff ? "Differences: " : "Demangled ") << count << " symbol(s)";
        if (gnu)
        {
            cout << " using GNU cplus_demangle()";
        }
        cout << endl;

        time_t timeEnd(time(0));
        cout << difftime(timeEnd, timeStart) << " seconds\n";
        cout << difftime(timeEnd, timeStart) / count << " sec/symbol\n";
        return 0;
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
    }
    return -1;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
