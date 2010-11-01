//
// $Id: unmangle.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <stdlib.h>
#include <dlfcn.h>
#include "config.h"
#include <iostream>
#include <new>          // for std::bad_alloc
#include "decoder.h"
#include "legacy.h"
#include "unmangle.h"

using namespace std;


/*
 * for plugging in external demanglers
 */
typedef char* DemangleProc(char*);
static DemangleProc* demangle_dlang = NULL;


static DemangleProc* import(const char* libname, const char* fname)
{
    DemangleProc* proc = NULL;

    if (void* handle = dlopen(libname, RTLD_LAZY))
    {
        proc = (DemangleProc*) dlsym(handle, fname);
    }
    return proc;
}


/**
 * Determine whether D Language is supported.
 */
static bool support_dlang(bool force = false)
{
    static bool once = true;

    if (once || force)
    {
        if (const char* var = getenv("ZERO_D_SUPPORT"))
        {
     #ifdef DEBUG
            clog << __func__ << "=" << var << endl;
     #endif
            if (strcmp(var, "1") == 0 || strcmp(var, "true") == 0)
            {
                demangle_dlang = import("libdemangle_d.so", "demangle_d");
            }
        }
    /* #ifdef DEBUG
        else
        {
            clog << __func__ << ": not set\n";
        }
    #endif */
        once = false;
    }
    return demangle_dlang != NULL;
}


/**
 * Wrapper or "driver" function around: modern demangler as described
 * in the Itanium ABI, legacy 2.95 GNU demangler, and D Language demangler.
 */
char* unmangle( const char* mangled,
                size_t*     buflen,
                int*        status,
                int         optionFlags)
{
    if (mangled)
    {
        try
        {
            const size_t length = buflen ? *buflen : 0;

            if (mangled[0] == '_' && mangled[1] == 'Z')
            {
                Decoder<char> decoder(mangled, length, optionFlags);
                return decoder.parse(status, buflen);
            }
            else if ((strncmp("_GLOBAL_", mangled, 8) == 0)
	            && (mangled[8] == '.' || mangled[8] == '_' || mangled[8] == '$')
	            && (mangled[9] == 'D' || mangled[9] == 'I' || mangled[9] == 'N'))
            {
	            switch (mangled[10])
                {
                case '_':
                    {
                        Decoder<char> d1(mangled, length, optionFlags);
                        return d1.parse_global_ctor_dtor(status, buflen);
                    }
                case '.':
                    {
                        legacy::Decoder<char> d2(mangled, optionFlags);
                        return d2.parse_global_ctor_dtor(status, buflen);
                    }
                }
                return 0;
            }
            else // fallback to legacy (g++ 2.95) mangling scheme
            {
                legacy::Decoder<char> decoder(mangled, optionFlags);
                if (char* result = decoder.parse(status, buflen))
                {
                    if (result[0] == '<' && result[1] == '>')
                    {
                        return 0;
                    }
                    return result;
                }
                else if (support_dlang() && demangle_dlang)
                {
                    if (char* result = demangle_dlang(const_cast<char*>(mangled)))
                    {
                        if (buflen)
                        {
                            *buflen = strlen(result);
                        }
                        return result;
                    }
                }
            }
        }
        catch (const bad_alloc&)
        {
            if (status)
            {
                *status = UNMANGLE_STATUS_BAD_ALLOC;
            }
        }
        catch (const exception& e)
        {
            if (status)
            {
                *status = UNMANGLE_STATUS_EXCEPTION;
            }
            cerr << __func__ << ": " << e.what() << endl;
        }
        catch (...)
        {
            if (status)
            {
                *status = UNMANGLE_STATUS_UNKNOWN_EXCEPTION;
            }
            cerr << __func__ << ": unknown exception\n";
        }
    }
    return 0;
}


void cplus_unmangle_d()
{
    support_dlang(true);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
