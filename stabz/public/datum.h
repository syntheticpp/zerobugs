#ifndef DATUM_H__249530F1_D519_4E6C_9AD1_18FDA56B9288
#define DATUM_H__249530F1_D519_4E6C_9AD1_18FDA56B9288
//
// $Id: datum.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/data_type.h"
#include "zdk/ref_counted_impl.h"
#include "zdk/shared_string.h"
#include "zdk/weak_ptr.h"
#include "stabz/public/object.h"


namespace Stab
{
    /**
     * Base class for variables, function parameters
     * and run-time constants.
     */
    class Datum : public RefCountedImpl<Object>
    {
    public:
        ~Datum() throw() {}

        SharedString& name() const;

        /**
         * @return the type of this datum.
         */
        WeakDataTypePtr type() const { return type_; }

    protected:
        Datum(SharedString&, DataType&);

    private:
        RefPtr<SharedString> name_;
        WeakDataTypePtr      type_;
    };
}
#endif // DATUM_H__249530F1_D519_4E6C_9AD1_18FDA56B9288
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
