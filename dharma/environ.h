#ifndef ENVIRON_H__CF130F5D_67AD_4833_8187_548BD5425090
#define ENVIRON_H__CF130F5D_67AD_4833_8187_548BD5425090
//
// $Id: environ.h 714 2010-10-17 10:03:52Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cstdlib>
#include <iosfwd>
#include <sstream>
#include <string>

class SArray;

namespace env
{
    bool get_bool(const char*, bool defaultValue = false);

    std::string get_string(const char*, const char* defaultStr = NULL);

    std::string inline get(const char* name, const char* defaultStr = NULL)
    { return get_string(name, defaultStr); }

    int get(const char*, int defaultValue = 0);

    double get(const char*, double defaultValue = .0);

    template<typename T>
    void set(const char* name, T value, bool overwrite = true)
    {
        std::ostringstream tmp;
        tmp << value;
        ::setenv(name, tmp.str().c_str(), overwrite);
    }

    /**
     * Read the environment from a file (which is expected to be
     * in the format of /proc/PID/environ files).
     * Store result in given SArray
     */
    void read(const std::string& filename, SArray&);

    void read(std::istream&, SArray&);

    void write(const std::string& filename, const char* const*);
}


#endif // ENVIRON_H__CF130F5D_67AD_4833_8187_548BD5425090
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
