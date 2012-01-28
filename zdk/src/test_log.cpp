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
#define ALLOW_ENDL 1
#include "zdk/eventlog.h"

using namespace std;

struct DebugChannel : public eventlog::Channel<DebugChannel>
{
    static void prefix(basic_ostream<char_type>& out)
    {
        out << "[debug] ";
    }
 /*
    void enter()
    {
        clog << __func__ << endl;
    }
    void leave()
    {
        clog << __func__ << endl;
    }
 */
};

int main()
{
    eventlog::Stream<DebugChannel>() << "Hello World" << endl;
    eventlog::Stream<DebugChannel>() << hex << 32 << eventlog::endl;

    eventlog::Stream<DebugChannel>(1) << "Goodbye World" << eventlog::endl;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
