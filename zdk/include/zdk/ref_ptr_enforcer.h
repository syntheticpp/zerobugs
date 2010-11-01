#ifndef REF_PTR_ENFORCER_H__3D98F443_97DD_4F39_B738_F36A18E20A8E
#define REF_PTR_ENFORCER_H__3D98F443_97DD_4F39_B738_F36A18E20A8E
//
// $Id: ref_ptr_enforcer.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdio.h>
#include "ref_counted.h"


#if DEBUG
/**
 * Used for detecting pointers to RefCounted, returned
 * from functions, that the caller code has failed to
 * wrap into a RefPtr smart pointer.
 */
class ZDK_LOCAL RefPtrEnforcer : public RefTracker
{
public:
    RefPtrEnforcer(const char* file, size_t line)
        : obj_(0), file_(file), line_(line)
    {
    }

    virtual ~RefPtrEnforcer()
    {
        if (obj_ && obj_->ref_count() == 0)
        {
            /*  If this happens, it is most likely because
                a RefCounted object that should've been
                wrapped into a RefPtr, wasn't. Dude, you're
                leaking! */

            fprintf(stderr,
                "%s line %ld: RefPtr<T>-wrapped pointer expected\n",
                file_, line_);
            abort();
        }
    }

    void register_object(RefCounted* obj)
    {
        assert(!obj_);

        if (obj)
        {
            obj_ = obj;
        }
    }

    /**
     * Silence off 'taking address of temporary' compiler warning.
     */
    RefPtrEnforcer* operator&() { return this; }

private:
    RefPtrEnforcer(const RefPtrEnforcer&);
    RefPtrEnforcer& operator=(const RefPtrEnforcer&);

    RefCounted* obj_;
    const char* const file_;
    const long line_;
};

#endif // DEBUG

#define ENFORCE_REF_PTR_ RefTracker* = 0

#endif // REF_PTR_ENFORCER_H__3D98F443_97DD_4F39_B738_F36A18E20A8E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
