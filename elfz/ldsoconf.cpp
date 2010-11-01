//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
//
// $Id: ldsoconf.cpp 711 2010-10-16 07:09:23Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include "dharma/path.h"
#include <glob.h>
#include <fstream>
#include <iostream>
#include <set>
#include <boost/tokenizer.hpp>
#include "dharma/canonical_path.h"
#include "private/ldsoconf.h"

using namespace std;

namespace
{
    typedef  boost::char_separator<char> Delim;
    typedef boost::tokenizer<Delim> Tokenizer;
}


/**
 * Parse ld.so.conf file and append all paths to dirs vector
 */
static void
_ldsoconf_parse(const char* filename,
                vector<string>& dirs,
                set<string>& includes)
{
    // avoid cyclical includes
    if (!includes.insert(canonical_path(filename)).second)
    {
        return;
    }
    ifstream f;

    f.exceptions(ios_base::badbit);
    f.open(filename);

    while (f)
    {
        vector<char> line(4096);
        f.getline(&line[0], line.size() - 1);

        string buf(&line[0]);
        // ld.so.conf is a
        // "File containing a list of colon,  space,  tab,  newline,
        // or  comma-separated directories in which to search for
        // libraries.";
        Tokenizer tok(buf, Delim(": \t\n,"));
        bool comment = false;
        for (Tokenizer::iterator i = tok.begin(); !comment && (i != tok.end()); )
        {
            if ((*i)[0] == '#')
            {
                comment = true; // skip the rest of the tokens
            }
            else
            {
                //
                // It is not documented in man ldconfig, but "include" is
                // a recognized keyword, and the argument(s) may contain wildcards.
                //
                if (*i == "include")
                {
                    for (++i; i != tok.end(); ++i)
                    {
                        glob_t gbuf = { 0 };

                        try
                        {
                            string pattern;

                            if ((*i)[0] == '/')
                            {
                                pattern = *i;
                            }
                            else
                            {
                                // assume same directory as current file
                                pattern = sys::dirname(filename) + "/" + *i;
                            }
                            if (glob(pattern.c_str(), GLOB_NOMAGIC, NULL, &gbuf) == 0)
                            {
                                for (size_t n = 0; n != gbuf.gl_pathc; ++n)
                                {
                                #if DEBUG
                                    // clog << "*** " << gbuf.gl_pathv[n] << " ***\n";
                                #endif
                                    _ldsoconf_parse(gbuf.gl_pathv[n], dirs, includes);
                                }
                            }
                        }
                        catch (...)
                        {
                            globfree(&gbuf);
                            throw;
                        }
                        globfree(&gbuf);
                    }
                }
                else
                {
                    dirs.push_back(*i++);
                }
            }
        }
    }
}


void ldsoconf_parse(const char* filename, vector<string>& dirs)
{
    set<string> uniqueFiles;
    _ldsoconf_parse(filename, dirs, uniqueFiles);
}

#ifdef TEST_LDSOCONF

int main()
{
    try
    {
        vector<string> dirs;

        //ldsoconf_parse("/etc/ld.so.conf", dirs);
        ldsoconf_parse("test.conf", dirs);
        copy(dirs.begin(), dirs.end(), ostream_iterator<string>(cout, "\n"));
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
    }
}

#endif
