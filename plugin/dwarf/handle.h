#ifndef HANDLE_H__A65A3DDA_C813_4CC7_BBDD_674069BA55F7
#define HANDLE_H__A65A3DDA_C813_4CC7_BBDD_674069BA55F7
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: handle.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "generic/lock_ptr.h"
#include "dwarfz/public/debug.h"


namespace Dwarf
{
    CLASS Handle
    {
        typedef boost::shared_ptr<Debug> DebugPtr;

    public:
        typedef LockedPtr<Debug, Debug, DebugPtr> pointer;

        Handle() { }

        explicit Handle(DebugPtr p)
        {
            if (p)
            {
                pptr_.reset(new pointer(p, *p));
            }
        }

        Handle& swap(Handle& other) throw()
        {
            pptr_.swap(other.pptr_);
            return *this;
        }

        Handle& operator=(DebugPtr p)
        {
            Handle tmp(p);
            return swap(tmp);
        }

        DebugPtr operator->() const { return pptr_->get(); }
        const Debug& operator*() const { return **pptr_; }
        Debug& operator*() { return **pptr_; }

        operator void*() const
        {
            return pptr_.get() ? pptr_->get().get() : NULL;
        }

        operator DebugPtr() const
        {
            return pptr_.get() ? pptr_->get() : DebugPtr();
        }

        const Debug* get() const
        {
            return pptr_.get() ? pptr_->get().get() : NULL;
        }

    private:
        boost::shared_ptr<pointer> pptr_;
    };
}

#endif // HANDLE_H__A65A3DDA_C813_4CC7_BBDD_674069BA55F7
