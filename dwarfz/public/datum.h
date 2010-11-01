#ifndef DATUM_H__D780537E_A741_405E_8E4D_2B9F35091B9C
#define DATUM_H__D780537E_A741_405E_8E4D_2B9F35091B9C
//
// $Id: datum.h 714 2010-10-17 10:03:52Z root $
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
#include <string>
#include "interface.h"
#include "die.h"
#include "global.h"
#include "zdk/ref_ptr.h"
#include "zdk/shared_string.h"

namespace Dwarf
{
    class ConstValue;
    class Location;
    class Type;

    /**
     * Base class for variables, parameters and constants.
     */
    CLASS Datum : public Die
    {
    public:
        boost::shared_ptr<Type> type() const;

        boost::shared_ptr<Location> loc(bool indirect = false) const;

        RefPtr<SharedString> linkage_name() const;

        /**
         * offset in bytes from the beginning of the enclosing scope
         */
        Dwarf_Off start_scope() const;

        void set_global(const boost::shared_ptr<Global>&);

        boost::shared_ptr<ConstValue> const_value() const;

    protected:
        virtual ~Datum() throw() {}

        Datum(Dwarf_Debug dbg, Dwarf_Die die);

        char* name_impl() const;

    private:
        boost::shared_ptr<Global> global_;
        mutable RefPtr<SharedString> linkageName_; // mangled symbol name
    };
}
#endif // DATUM_H__D780537E_A741_405E_8E4D_2B9F35091B9C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
