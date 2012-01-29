//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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

#include <assert.h>
#include <iostream>
#include "dharma/path.h"
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "debugger_base.h"
#include "step_over_mgr.h"

using namespace std;


StepOverManager::StepOverManager(DebuggerBase* debugger)
    : debugger_(debugger)
    , readingProperties_(false)
    , count_(0)
{
    assert(debugger_);
}


void StepOverManager::add_step_over(SharedString* file, long line)
{
    if (file)
    {
        dbgout(0) << __func__ << ": " << file->c_str() << ":" << line << endl;

        if (line == -1)
        {
            if (readingProperties_)
            {
                stepOverDirSet_.insert(file);
            }
            else
            {
                const string dir = sys::dirname(file->c_str());
                stepOverDirSet_.insert(shared_string(dir));
            }
        }
        else
        {
            RefPtr<SharedString> fileName(file);
            if (line == 0)
            {
                stepOverMap_.erase(fileName);
            }
            const StepOverMapRange r = stepOverMap_.equal_range(file);
            for (StepOverMap::iterator i = r.first; i != r.second; ++i)
            {
                if (i->second == 0)
                {
                    return; // already stepping over ALL functions in file
                }
                if (i->second == line)
                {
                    return; // already stepping over this line
                }
            }
            stepOverMap_.insert(make_pair(fileName, line));
        }
        update_properties();
    }
}


void StepOverManager::remove_step_over(SharedString* file, long line)
{
    if (file)
    {
        dbgout(0) << __func__ << ": " << file->c_str() << ":" << line << endl;
        if (line == 0)
        {
            stepOverMap_.erase(file);
            assert(stepOverMap_.count(file) == 0);
        }
        else if (line == -1)
        {
            stepOverDirSet_.erase(file);
            assert(stepOverDirSet_.count(file) == 0);
        }
        else
        {
            StepOverMapRange r = stepOverMap_.equal_range(file);
            for (; r.first != r.second; )
            {
                if (r.first->second == static_cast<size_t>(line))
                {
                    stepOverMap_.erase(r.first++);
                }
                else
                {
                    ++r.first;
                }
            }
        }
    }
    else
    {
        dbgout(0) << __func__ << ": clearing all entries" << endl;

        stepOverMap_.clear();
        stepOverDirSet_.clear();
    }
    update_properties();
}



size_t StepOverManager::enum_step_over(EnumCallback2<SharedString*, long>* cb) const
{
    size_t count = 0;

    StepOverMap::const_iterator i = stepOverMap_.begin();
    for (; i != stepOverMap_.end(); ++i)
    {
        if (cb)
        {
            cb->notify(i->first.get(), i->second);
        }
        ++count;
    }
    StepOverDirSet::const_iterator j = stepOverDirSet_.begin();
    for (; j != stepOverDirSet_.end(); ++j)
    {
        if (cb)
        {
            cb->notify(j->get(), -1);
        }
        ++count;
    }
    return count;
}


/**
 * Given a symbol, return true if the debugger should always step over it,
 * or false if it's okay to step in.
 */
bool StepOverManager::query_step_over(const SymbolMap* symbols, addr_t pc) const
{
    if (stepOverMap_.empty() && stepOverDirSet_.empty())
    {
        return false;
    }
    if (RefPtr<Symbol> sym = symbols->lookup_symbol(pc))
    {
        StepOverMapConstRange r = stepOverMap_.equal_range(sym->file());

        for (; r.first != r.second; ++r.first)
        {
            // this matches a function's first line only, which should
            // be fine since C++ does not allow to call into the middle
            // of a function
            if ((r.first->second == 0) || (r.first->second == sym->line()))
            {
                return true;
            }
        }
        const string dir = sys::dirname(CHKPTR(sym->file())->c_str());
        if (stepOverDirSet_.count(shared_string(dir))
         || stepOverDirSet_.count(sym->file()))
        {
            return true;
        }
    }
    return false;
}



/**
 * Used when de-persisting saved configurations / preferences from disc
 */
void StepOverManager::restore_from_properties()
{
    Temporary<bool> setFlag(readingProperties_, true);

    word_t count = properties()->get_word("step_over", 0);
    dbgout(1) << __func__ << ": count=" << count << endl;

    for (properties()->set_word("step_over", 0) ; count > 0; --count)
    {
        ostringstream name;
        name << "step_over_" << count;
        string val = properties()->get_string(name.str().c_str(), "");
        const char* p = 0;
        const long n = strtol(val.c_str(), const_cast<char**>(&p), 0);

        dbgout(1) << __func__ << ": " << p + 1 << " " << n << endl;
        if (*++p)
        {
            add_step_over(shared_string(p).get(), n);
        }
    }
    sync();
}


Properties* StepOverManager::properties()
{
    return CHKPTR(debugger_->properties());
}


void StepOverManager::sync()
{
    count_ = 0;
    enum_step_over(this); // see notify() below
    properties()->set_word("step_over", count_);
}


void StepOverManager::update_properties()
{
    if (!readingProperties_)
    {
        RefPtr<Properties> prop = properties();
        //
        // 1st pass: remove all
        //
        word_t count = prop->get_word("step_over", 0);
        for (; count > 0; --count)
        {
            ostringstream name;
            name << "step_over_" << count;
            prop->set_string(name.str().c_str(), NULL);
        }
        //prop->set_word("step_over", 0);
        //
        // 2nd pass: add to prop
        //
        sync();
    }
}


/**
 * Invoked indirectly from enum_step_over call above
 */
void StepOverManager::notify(SharedString* path, long line)
{
    ostringstream name, val;
    name << "step_over_" << ++count_;
    val << line << "!" << CHKPTR(path)->c_str();
    properties()->set_string(name.str().c_str(), val.str().c_str());
    dbgout(0) << __func__ << ": " << name.str() << ": " << val.str() << endl;
}
