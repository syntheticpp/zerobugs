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
#include "private/factory.h"
#include "public/error.h"
#include "public/type.h"
#include "public/impl.h"
#include "private/log.h"
#include "private/type_attr.h"

using namespace std;
using namespace Dwarf;


TypeAttr::TypeAttr(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attr)
    : Attribute(dbg, die, attr)
{
}


std::shared_ptr<Type> TypeAttr::value() const
{
    std::shared_ptr<Type> type;

    Dwarf_Off   off = 0;
    Dwarf_Error err = 0;
    Dwarf_Die   die = 0;

    switch (this->form())
    {
    case DW_FORM_ref1:
    case DW_FORM_ref2:
    case DW_FORM_ref4:
    case DW_FORM_ref8:
    case DW_FORM_ref_addr:

        if (dwarf_global_formref(attr(), &off, &err) == DW_DLV_ERROR)
        {
            THROW_ERROR(dbg(), err);
        }
        if (dwarf_offdie(dbg(), off, &die, &err) == DW_DLV_ERROR)
        {
            // report error but don't throw, to appease buggy DMD compiler
            cerr << Error::Message(dbg(), err, __FILE__, __LINE__) << endl;
            return type;
        }
        if (die)
        {
            type = std::dynamic_pointer_cast<Type>(Factory::instance().create(dbg(), die));
        }
        break;

    case DW_FORM_addr:
        if (dwarf_formaddr(attr(), &off, &err) == DW_DLV_ERROR)
        {
            THROW_ERROR(dbg(), err);
        }
        if (dwarf_offdie(dbg(), off, &die, &err) == DW_DLV_ERROR)
        {
            THROW_ERROR(dbg(), err);
        }
        if (die)
        {
            type = std::dynamic_pointer_cast<Type>(Factory::instance().create(dbg(), die));
        }
        break;

    // todo: other cases here

    default:
        dbgout(Log::ALWAYS) << "Unhandled form=0x" << hex << form() << dec << endl;
        assert(false);
    }

    return type;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
