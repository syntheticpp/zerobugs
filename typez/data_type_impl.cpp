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

#include <unistd.h>             // close
#include "zdk/shared_string_impl.h"
#include "public/data_type_impl.h"

#if 0
#include "zdk/mutex.h"

static Mutex& get_mutex()
{
    static Mutex mx;
    return mx;
}
static Mutex* _init_mutex = &get_mutex();

 #define SYNCHRONIZED Lock<Mutex> lock(get_mutex())
#else
 #define SYNCHRONIZED
#endif

using namespace std;
using namespace Platform;


SharedString* void_type()
{
    SYNCHRONIZED;

    static const RefPtr<SharedString> name(shared_string("void"));
    return name.get();
}


SharedString* unnamed_type()
{
    SYNCHRONIZED;

    static const RefPtr<SharedString> name(shared_string("<unnamed>"));
    return name.get();
}


SharedString* unnamed_union()
{
    SYNCHRONIZED;

    static const RefPtr<SharedString> name(shared_string("<unnamed union>"));
    return name.get();
}


DataType::~DataType()
{
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
