#ifndef STEP_OVER_MGR_H__9F679E5D_8FFB_4F3C_8D1D_C823D310B2B7
#define STEP_OVER_MGR_H__9F679E5D_8FFB_4F3C_8D1D_C823D310B2B7
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: step_over_mgr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <set>
#include <map>
#include "zdk/enum.h"
#include "zdk/platform.h"
#include "zdk/shared_string.h"
#include "zdk/weak_ptr.h"

using Platform::addr_t;

class DebuggerBase;
class SymbolMap;


/**
 * It is useful to always step over function calls.
 * Consider these lines for example:
 *
 * @code
 *  shared_ptr<Order> order = get_customer_order();
 *  double tax = compute_tax(order->total(), order->shipping_addr());
 * @endcode
 *
 *  When stepping into the second line, the user's intent is very
 *  likely to step into the body of compute_tax; but the debugger
 *  will only do so after stepping into shared_ptr::operator->() twice.
 *
 *  To avoid this behavior, the UI allows the user to designate functions,
 *  files, and directories that contain code which should always be stepped
 *  over.
 *
 *  This class manages the information associated with this feature.
 */
class StepOverManager : EnumCallback2<SharedString*, long>
{
    typedef std::set<RefPtr<SharedString> > StepOverDirSet;
    typedef std::multimap<RefPtr<SharedString>, unsigned int> StepOverMap;

    typedef std::pair<StepOverMap::iterator,
                      StepOverMap::iterator> StepOverMapRange;
    typedef std::pair<StepOverMap::const_iterator,
                      StepOverMap::const_iterator> StepOverMapConstRange;
public:
    explicit StepOverManager(DebuggerBase*);

    /**
     * Add a filename and line number that should always be stepped over.
     * This allows the user to step over inlined library code, such as
     * boost or STL templates.
     * @param file filename
     * @param lineNum if 0, step over all functions in given file,
     *  if -1, step over all function in directory, otherwise step
     *  over functions that match the line exactly.
     */
    void add_step_over(SharedString* file, long lineNum);

    /**
     * Remove step-over entry. If file is NULL, remove all entries.
     */
    void remove_step_over(SharedString* file, long lineNum);

    /**
     * Enumerate step-over entries, return count
     */
    size_t enum_step_over(EnumCallback2<SharedString*, long>*) const;

    /**
     * @return true if given filename and line match an existing entry
     */
    bool query_step_over(const SymbolMap*, addr_t progCount) const;

    void restore_from_properties();

private:
    void sync();
    void update_properties();
    void notify(SharedString*, long); // used by update_properites

    Properties* properties();
    DebugChannel debug_channel(const char* fn) const;

private:
    DebuggerBase*       debugger_;
    StepOverMap         stepOverMap_;
    StepOverDirSet      stepOverDirSet_;
    bool                readingProperties_;
    size_t              count_; // user by update_properites

};

#endif // STEP_OVER_MGR_H__9F679E5D_8FFB_4F3C_8D1D_C823D310B2B7
