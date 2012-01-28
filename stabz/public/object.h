#ifndef OBJECT_H__EAC97716_153B_4ADD_8F57_2112A9E0BE79
#define OBJECT_H__EAC97716_153B_4ADD_8F57_2112A9E0BE79
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

#include "zdk/zobject.h"

namespace Stab
{
    /**
     * A base class for the Stab hierarchy.
     */
    class ZDK_LOCAL Object : public ZObject
    {
    public:
        BEGIN_INTERFACE_MAP(Object)
            // empty
        END_INTERFACE_MAP()

        static size_t instances() { return instances_; }

    protected:
        Object();
        Object(const Object&);

        virtual ~Object();

        Object& operator=(const Object&);

    private:
        static size_t instances_;
    };
}
#endif // OBJECT_H__EAC97716_153B_4ADD_8F57_2112A9E0BE79
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
