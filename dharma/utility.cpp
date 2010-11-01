//
// $Id: utility.cpp 710 2010-10-16 07:09:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#ifdef DEBUG
#include <iostream>
#endif
#include <vector>
#include <boost/tokenizer.hpp>
#include "canonical_path.h"
#include "utility.h"

using namespace std;



static void tokenize(const string& pathlist, vector<string>& v)
{
    typedef  boost::char_separator<char> Delim;
    typedef boost::tokenizer<Delim> Tokenizer;

    Tokenizer tok(pathlist, Delim(":;"));
    v.insert(v.end(), tok.begin(), tok.end());
}


static void inline append_delimiter(string& path)
{
    if (!path.empty() && path[path.size() - 1] != '/')
    {
        path += '/';
    }
}


void ensure_abs_lib_path(const char* const* env, string& path)
{
    static const char ld_library_path[] = "LD_LIBRARY_PATH=";
    static const char ld_run_path[] = "LD_RUN_PATH=";

    string workDirectory;
    vector<string> searchPath;

    if (!path.empty() && path[0] != '/')
    {
        for (;env && *env; ++env)
        {
            if (!strncmp(*env, ld_library_path, sizeof ld_library_path - 1))
            {
                tokenize(*env + sizeof ld_library_path - 1, searchPath);
            }
            else if (!strncmp(*env, ld_run_path, sizeof ld_run_path - 1))
            {
                tokenize(*env + sizeof ld_run_path - 1, searchPath);
            }
            else if (!strncmp(*env, "PWD=", 4))
            {
                workDirectory = *env + 4;
                searchPath.push_back(workDirectory);
            }
        }
        if (workDirectory.empty())
        {
            char buf[PATH_MAX];
            memset(buf, 0, sizeof buf);
            getcwd(buf, sizeof buf - 1);
            workDirectory = buf;
        }
        size_t n = path.rfind('/');
        if (n != path.npos)
        {
            path = path.substr(n + 1);
        }

        vector<string>::const_iterator i = searchPath.begin();
        for (; i != searchPath.end(); ++i)
        {
            if (i->empty())
            {
                continue;
            }
            string tmp;

            // assume it is relative to the working dir
            if ((*i)[0] == '.' && (*i) != workDirectory)
            {
                tmp = workDirectory;
                append_delimiter(tmp);
            }
            tmp += *i;
            append_delimiter(tmp);
            tmp += path;
            tmp = canonical_path(tmp.c_str());

            struct stat finfo;
            if (stat(tmp.c_str(), &finfo) == 0 && S_ISREG(finfo.st_mode))
            {
            #ifdef DEBUG
                clog << __func__<< ": " << path << " --> " << tmp << endl;
            #endif
                path = tmp;
                break;
            }
        }
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
