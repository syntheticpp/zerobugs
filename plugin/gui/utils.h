#ifndef UTILS_H__12070183_E180_4548_B733_E94F4285BFE5
#define UTILS_H__12070183_E180_4548_B733_E94F4285BFE5
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: utils.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <gdk/gdk.h>
#include "zdk/export.h"
#include "zdk/history.h"
#include "zdk/ref_ptr.h"
#include "zdk/zero.h"
#include "generic/auto.h"


inline RefPtr<Properties> ZDK_LOCAL get_history_properties(Thread& thread)
{
    RefPtr<Process> process = thread.process();
    RefPtr<Properties> props;

    if (Debugger* debugger = thread.debugger())
    {
        props = debugger->properties();
    }
    if (process && props)
    {
        RefPtr<History> hist = interface_cast<History*>(props->get_object("hist"));

        if (hist)
        {
            RefPtr<HistoryEntry> entry = hist->get_entry(process->name());
            return interface_cast<Properties>(entry);
        }
    }
    return NULL;
}


struct ZDK_LOCAL ThreadsGuard : Automatic
{
    ThreadsGuard() { gdk_threads_enter(); }
    ~ThreadsGuard() { gdk_threads_leave(); }
};

#endif // UTILS_H__12070183_E180_4548_B733_E94F4285BFE5
