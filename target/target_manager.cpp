//
// $Id: target_manager.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <assert.h>
#include "process.h"
#include "target_manager.h"

using namespace std;


////////////////////////////////////////////////////////////////
TargetManager::TargetManager() : iterCount_(0)
{
}


////////////////////////////////////////////////////////////////
TargetManager::~TargetManager()
{
    assert(iterCount_ == 0);
}


////////////////////////////////////////////////////////////////
void
TargetManager::add_target_internal(const RefPtr<Target>& ptr)
{
    mutex_.assert_locked();
    assert(iterCount_ == 0);
/*#ifdef DEBUG
    if (Process* proc = ptr->process())
    {
        clog << __func__ << ": " << proc->pid() << endl;
    }
#endif */
    Container::iterator i = find(targets_.begin(), targets_.end(), ptr);

    if (i == targets_.end())
    {
        targets_.push_back(ptr);
    }
}


////////////////////////////////////////////////////////////////
void
TargetManager::remove_target_internal(const RefPtr<Target>& ptr)
{
    mutex_.assert_locked();
    assert(iterCount_ == 0);

    Container::iterator i(find(targets_.begin(), targets_.end(), ptr));

    if (i != targets_.end())
    {
/* #ifdef DEBUG
        if (Process* proc = (*i)->process())
        {
            clog << __func__ << ": " << proc->pid() << endl;
        }
#endif */
        targets_.erase(i);
    }
}


////////////////////////////////////////////////////////////////
void TargetManager::add_target(const RefPtr<Target>& target)
{
    Lock<Mutex> lock(mutex_);
    if (iterCount_ == 0) // no outstanding iterator?
    {
        add_target_internal(target);
    }
    else
    {
        pendingAddition_.push_back(target);
    }
}


////////////////////////////////////////////////////////////////
void TargetManager::remove_target(const RefPtr<Target>& target)
{
    Lock<Mutex> lock(mutex_);
    if (iterCount_ == 0)
    {
        remove_target_internal(target);
    }
    else
    {
        pendingRemoval_.push_back(target);
    }
}


////////////////////////////////////////////////////////////////
void TargetManager::decrement_iter_count()
{
    Lock<Mutex> lock(mutex_);
    assert(iterCount_);

    if (--iterCount_ == 0)
    {
        Container::iterator i = pendingAddition_.begin();
        while (i != pendingAddition_.end())
        {
            add_target_internal(*i);
            i = pendingAddition_.erase(i);
        }

        i = pendingRemoval_.begin();
        while (i != pendingRemoval_.end())
        {
            remove_target_internal(*i);
            i = pendingRemoval_.erase(i);
        }
    }
}


////////////////////////////////////////////////////////////////
void TargetManager::remove_all()
{
    Lock<Mutex> lock(mutex_);
    assert(!iterCount_);
    assert(pendingAddition_.empty());
    assert(pendingRemoval_.empty());

    targets_.clear();
}


////////////////////////////////////////////////////////////////
RefPtr<Target> TargetManager::find_target(pid_t lwpid) const
{
    RefPtr<Target> result;

    Lock<Mutex> lock(mutex_);
    Container::const_iterator i = targets_.begin();
    for (; i != targets_.end(); ++i)
    {
        const RefPtr<Target>& target = *i;

        if (Process* proc = target->process())
        {
            // if the process matches the lwpid, it's our target
            if (proc->pid() == lwpid)
            {
                // but first, make sure that it has any threads,
                // because otherwise it means that the prrocess is
                // on its way to the great beyond
                if (target->enum_threads())
                {
                    result = target;
                }
                break;
            }
        }
        if (target->get_thread(lwpid, 0))
        {
            result = target;
            break;
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
TargetManager::iterator TargetManager::begin(const Lock<Mutex>& lock)
{
    assert(lock.holds(mutex_));
    return iterator(*this, targets_.begin());
}


////////////////////////////////////////////////////////////////
TargetManager::iterator TargetManager::end(const Lock<Mutex>& lock)
{
    assert(lock.holds(mutex_));
    return iterator(*this, targets_.end());
}


////////////////////////////////////////////////////////////////
TargetManager::const_iterator
TargetManager::begin(const Lock<Mutex>& lock) const
{
    assert(lock.holds(mutex_));
    return targets_.begin();
}


////////////////////////////////////////////////////////////////
TargetManager::const_iterator
TargetManager::end(const Lock<Mutex>& lock) const
{
    assert(lock.holds(mutex_));
    return targets_.end();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
