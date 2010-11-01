//
// $Id: dynamic_lib_unix.cpp 714 2010-10-17 10:03:52Z root $
//
// Contains the UNIX-specific part of the DynamicLib class impl.
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <dlfcn.h>

#include "dynamic_lib.h"
#include "environ.h"
#include "process_name.h"


using namespace std;

//
// NODLCLOSE is useful when debugging with Valgrind, so that
// we get meaningful stack traces
//
static bool leak = env::get_bool("NODLCLOSE", false);

////////////////////////////////////////////////////////////////
void DynamicLibHandleTraits::dispose(void*& handle) throw()
{
    if (handle && !leak)
    {
        int res = ::dlclose(handle);

        if (res == 0)
        {
    #if DEBUG
            clog << "Unloaded: " << hex << handle << dec << endl;
    #endif
            handle = 0;
        }
    #if DEBUG
        else
        {
            clog << ::dlerror() << " " << hex << handle << dec;
            clog << ": result=" << res << endl;
        }
    #endif
    }
}


////////////////////////////////////////////////////////////////
void DynamicLib::load()
{
    // dynamic linker binding flags
    static const int flags = RTLD_LAZY | RTLD_GLOBAL;
    // static const int flags = RTLD_NOW | RTLD_GLOBAL;
    // static const int flags = RTLD_LAZY;

    if (handle_)
    {
        // clog << "already loaded: " << file_ << endl;
    }
    else
    {
        const char* fName = is_self() ? 0 : filename().c_str();
        handle_.reset(::dlopen(fName, flags));

        if (!handle_)
        {
            if (const char* err = ::dlerror())
            {
                throw runtime_error(err);
            }
            else
            {
                throw runtime_error("Unknown dlopen error");
            }
        }
    //#if DEBUG
      //  clog << "Loaded: " << filename();
      //  clog << " (handle=" << hex << handle_.get() << dec << ")\n";
    // #endif
    }
}


////////////////////////////////////////////////////////////////
void* DynamicLib::address_of(const char* symbolName)
{
    void* result = 0;

    load(); // ensure that the module is loaded

    assert(handle_);
    result = ::dlsym(handle_.get(), symbolName);

    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
