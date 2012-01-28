#ifndef TARGET_MANAGER_H__AA71A11A_F0F3_4207_891E_6085F8F77BA5
#define TARGET_MANAGER_H__AA71A11A_F0F3_4207_891E_6085F8F77BA5
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
//
#include <vector>
#include "iterator.h"
#include "target.h"


/**
 * Manages a collection of targets.
 *
 * Typically, the debugger is attached to one target, but sometimes
 * a process can fork()/exec(), and if the debugger is set up to
 * automatically trace the newly create process, then we have
 * a new target.
 */
class TargetManager
{
public:
    typedef std::vector<RefPtr<Target> > Container;
    typedef RefCountedIterator<Target, TargetManager, Container> iterator;
    typedef Container::const_iterator const_iterator;

    friend class RefCountedIterator<Target, TargetManager, Container>;

    TargetManager();
    virtual ~TargetManager();

    void add_target(const RefPtr<Target>&);
    void remove_all();

    /**
     * Find the target where the process or thread, as given
     * by the liteweight process id, belongs
     */
    RefPtr<Target> find_target(pid_t lwpid) const;

    iterator begin(const Lock<Mutex>&);
    iterator end(const Lock<Mutex>&);

    const_iterator begin(const Lock<Mutex>&) const;
    const_iterator end(const Lock<Mutex>&) const;

    bool empty(const Lock<Mutex>& lock) const
    {
        assert(lock.holds(mutex_));
        return targets_.empty();
    }
    bool empty() const
    {
        Lock<Mutex> lock(mutex_);
        return targets_.empty();
    }
    size_t size() const
    {
        Lock<Mutex> lock(mutex_);
        return targets_.size();
    }
    Mutex& mutex() const { return mutex_; }

protected:
    void remove_target(const RefPtr<Target>&);

    void erase(iterator i)
    {
        mutex_.assert_locked();
        targets_.erase(i.base_iterator());
    }

private:
    void add_target_internal(const RefPtr<Target>&);
    void remove_target_internal(const RefPtr<Target>&);

    // manage count of outstanding iterators
    void increment_iter_count() { Lock<Mutex> lock(mutex_); ++iterCount_; }
    void decrement_iter_count();

    long        iterCount_;
    Container   targets_;
    Container   pendingAddition_;
    Container   pendingRemoval_;

    mutable Mutex mutex_;
};

#endif // TARGET_MANAGER_H__AA71A11A_F0F3_4207_891E_6085F8F77BA5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
