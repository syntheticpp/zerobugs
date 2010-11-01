#ifndef CPU_STATE_SAVER_H__7B0325F4_5E77_4910_8F59_59A04F7D7D30
#define CPU_STATE_SAVER_H__7B0325F4_5E77_4910_8F59_59A04F7D7D30
//
// $Id: cpu_state_saver.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/export.h"
/**
 * Save/restore CPU state when calling functions in
 * the debugged program via the expression interpreter
 */
class ZDK_LOCAL CPUStateSaver
{
public:
    explicit CPUStateSaver(Thread& currentThread);

    ~CPUStateSaver() throw();

    /**
     * restore registers and pending signals
     */
    void restore_state();

protected:
    const Thread* thread() const { return thread_.get(); }

private:
    CPUStateSaver(const CPUStateSaver&);
    CPUStateSaver& operator=(const CPUStateSaver&);

    RefPtr<Thread>  thread_; // todo: should it be WeakPtr?
    bool            restored_;
    int             signal_;
};
#endif // CPU_STATE_SAVER_H__7B0325F4_5E77_4910_8F59_59A04F7D7D30
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
