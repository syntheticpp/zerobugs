#ifndef PROGVIEW_SIGNALS_H__7CB94167_8BAD_4EC9_A07E_4C91866CEB8E
#define PROGVIEW_SIGNALS_H__7CB94167_8BAD_4EC9_A07E_4C91866CEB8E
//
// $Id: prog_view_signals.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "code_view_signals.h"



struct ProgramViewSignals : public CodeViewSignals
{
    /// Signals that are emitted when history_back() and
    /// history_forward() are available (so that we
    /// can enable/disable the toolbar buttons).
    SigC::Signal1<void, size_t> history_can_back;
    SigC::Signal1<void, size_t> history_can_forward;
    /// </files navigation>

    SigC::Signal1<void, RefPtr<Frame> > show_frame;

    SigC::Signal0<void> views_changed;
};

#endif // PROGVIEW_SIGNALS_H__7CB94167_8BAD_4EC9_A07E_4C91866CEB8E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
