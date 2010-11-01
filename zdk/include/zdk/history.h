#ifndef HISTORY_H__8DA9AEF0_BBFD_4D0A_824C_40F4AD730821
#define HISTORY_H__8DA9AEF0_BBFD_4D0A_824C_40F4AD730821
//
// $Id: history.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zobject.h"
#include "zdk/process.h"

/**
 * A history entry corresponds to a program that has
 * been debugged.
 */
DECLARE_ZDK_INTERFACE_(HistoryEntry, ZObject)
{
    DECLARE_UUID("43fedd23-4b71-4fbf-b9fb-9751859ceb29")

    virtual pid_t pid() const = 0;

    virtual const char* name() const = 0;

    virtual time_t last_debugged() const = 0;

    virtual const char* command_line() const = 0;

    virtual const char* const* environ() const = 0;

    /**
     * @return true for live processes, false for core dumps.
     */
    virtual bool is_live() const = 0;

    /**
     * @return the optional target-specific param that was
     * passed in as the second parameter of Debugger::attach,
     * or empty string.
     * @note clients should check for NULL as well as empty string.
     */
    virtual const char* target_param() const = 0;

    virtual void set_environ(const char* const*) = 0;
    virtual void set_command_line(const char*) = 0;

    virtual ProcessOrigin origin() const = 0;
};


/**
 * A collection of history entries.
 */
DECLARE_ZDK_INTERFACE_(History, ZObject)
{
    DECLARE_UUID("8c2adcb5-35e5-4fe7-9cd6-44e622c98117")

    /**
     * number of entries in history
     */
    virtual size_t entry_count() const = 0;

    /**
     * get entry by index
     * @throw out_of_range
     */
    virtual const HistoryEntry* entry(size_t) const = 0;

    virtual void remove_entry(size_t) = 0;

    /**
     * get entry by name
     */
    virtual HistoryEntry* get_entry(const char*) const = 0;
};

#endif // HISTORY_H__8DA9AEF0_BBFD_4D0A_824C_40F4AD730821
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
