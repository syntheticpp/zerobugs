#ifndef DBGOUT_H__482064A4_E0C2_4628_B7B4_CED236F198F0
#define DBGOUT_H__482064A4_E0C2_4628_B7B4_CED236F198F0
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

#include <string>
#include "zdk/eventlog.h"

#ifdef dbgout
 #undef dbgout
#endif


namespace Dwarf
{
    extern int debugLevel_;

    class ZDK_LOCAL DebugChannel : public eventlog::Channel<DebugChannel>
    {
        std::string file_;
        size_t line_;

    public:
        DebugChannel(const char* file, size_t line)
            : file_(file)
            , line_(line)
        { }

        int level() const { return debugLevel_; }

        void prefix(std::ostream& outs) const
        {
            outs << "dwarf (" << file_ << ':' << line_ << "): ";
        }
    };

    typedef eventlog::Stream<DebugChannel> DebugStream;

    inline DebugChannel
    ZDK_LOCAL debug_channel(const char* file, size_t line)
    {
        return DebugChannel(file, line);
    }
    inline DebugStream
    ZDK_LOCAL debug_stream(const char* file, size_t line, int level)
    {
        return DebugStream(debug_channel(file, line), level);
    }
}


using namespace eventlog;
#ifdef DEBUG

 #define dbgout(n) Dwarf::debug_stream(__FILE__, __LINE__, (n))
#else
 #define dbgout(level) eventlog::Null<>()
#endif

#endif // DBGOUT_H__482064A4_E0C2_4628_B7B4_CED236F198F0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
