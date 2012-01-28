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
#if 0 // defined(DEBUG_OBJECT_LEAKS)
#include <execinfo.h>
#include "generic/singleton.h"
#include "generic/temporary.h"
#include "zdk/ref_ptr.h"

bool collecting = false;

////////////////////////////////////////////////////////////////
Traced::Traced() : obs_(this)
{
    if (!collecting)
    {
        trace_.resize(64);
        int size = ::backtrace(&trace_[0], trace_.size());

        if (size >= 0)
        {
            trace_.resize(size);
        }

        Singleton<TraceCollector>::instance().attach(&obs_);
    }
}


////////////////////////////////////////////////////////////////
Traced::~Traced() throw()
{
}


////////////////////////////////////////////////////////////////
void Traced::on_state_change(Subject* subj)
{
    if (TraceCollector* tc = interface_cast<TraceCollector*>(subj))
    {
        tc->collect(trace_);
    }
}


////////////////////////////////////////////////////////////////
TraceCollector::~TraceCollector() throw()
{
}

void (*collect_trace)(const std::vector<void*>&) = 0;

////////////////////////////////////////////////////////////////
void TraceCollector::collect(const std::vector<void*>& trace)
{
    Temporary<bool> set_flag(collecting, true);

    if (collect_trace)
    {
        (*collect_trace)(trace);
    }
}

#endif // DEBUG_OBJECT_LEAKS
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
