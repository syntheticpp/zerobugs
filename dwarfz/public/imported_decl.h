#ifndef IMPORTED_DECL_H__15773532_B655_45F3_9112_A7BF901E6667
#define IMPORTED_DECL_H__15773532_B655_45F3_9112_A7BF901E6667
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

#include <dwarf.h>
#include "child.h"
#include "die.h"
#include "namespace.h"


namespace Dwarf
{
    CLASS ImportedDecl : public Die, public Child<Namespace>
    {
    public:
        enum { TAG = DW_TAG_imported_declaration };

        ImportedDecl(Dwarf_Debug, Dwarf_Die);

        virtual ~ImportedDecl() throw () { }

        boost::shared_ptr<Die> get_import();
    };
}


#endif // IMPORTED_DECL_H__15773532_B655_45F3_9112_A7BF901E6667
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
