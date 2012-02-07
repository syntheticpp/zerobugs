#ifndef SUBROUTINE_TYPE_H__C2E581D7_2304_4334_8E4B_EC1791831FF5
#define SUBROUTINE_TYPE_H__C2E581D7_2304_4334_8E4B_EC1791831FF5
//
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
#include <dwarf.h>
#include "child.h"
#include "interface.h"
#include "list.h"
#include "parameter.h"
#include "type.h"

namespace Dwarf
{
    CLASS SubroutineType : public Type
    {
        DECLARE_CONST_VISITABLE();

    public:
        typedef ParameterT<SubroutineType> Param;

    public:
        enum { TAG = DW_TAG_subroutine_type };

        /* the subroutine' s return type */
        std::shared_ptr<Type> ret_type() const;

        List<Parameter> params() const;

        /* todo: support DW_TAG_unspecified_parameters */

    protected:
        SubroutineType(Dwarf_Debug, Dwarf_Die);

    };
}

#endif // SUBROUTINE_TYPE_H__C2E581D7_2304_4334_8E4B_EC1791831FF5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
