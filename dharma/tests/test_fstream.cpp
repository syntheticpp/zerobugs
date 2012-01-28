//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
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
#include <math.h>
#include <string.h>
#include <boost/limits.hpp>
#include "dharma/fstream.h"


static word_t w = 1234;
static double d = 1234.5678;
static const char s[] = "Hello World";


/**
 * Receives name-value pair notifications from the InputStream.
 */
class ZDK_LOCAL MyStreamEvents : public InputStreamEvents
{
    virtual void on_word(const char* name, word_t val)
    {
        assert(strcmp(name, "test_word") == 0);
        assert(val == w);
    }

    virtual void on_double(const char* name, double val)
    {
        assert(strcmp(name, "test_double") == 0);
        assert(fabs(d - val)  < std::numeric_limits<double>::epsilon());
    }

    virtual void on_string(const char* name, const char* val)
    {
        assert(strcmp(name, "test_string") == 0);
        assert(strcmp(val, s) == 0);
    }

    virtual size_t on_object_begin(
            InputStream*,
            const char* name,
            uuidref_t   uuid,
            size_t      size)
    { assert(false); return 0; }

    virtual void on_opaque(const char*, uuidref_t, const uint8_t*, size_t) { assert(false); }

    virtual void on_object_end() { assert(false); }

    virtual void on_bytes( const char* name,
                           const void* buf,
                           size_t buflen)
    { assert(false); }
};


int main()
{
    try
    {
        { // write file
            FileStream out("test.bin");

            out.write_word("test_word", w);
            out.write_double("test_double", d);
            out.write_string("test_string", s);
        }
        { // read it back
            MyStreamEvents events;

            FileStream in("test.bin", true); // readonly

            while (in.read(&events))
            {
            }
        }
    }
    catch (...)
    {
        assert(false);
    }
    return 0;
}

