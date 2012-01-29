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
#include "error.h"
#include "private/log.h"

#include "impl.h"
#include "comp_dir_attr.h"

using namespace std;
using namespace Dwarf;


CompDirAttr::CompDirAttr(Dwarf_Debug dbg, Dwarf_Die die)
    : Attribute(dbg, die, DW_AT_comp_dir)
{
}


const char* CompDirAttr::value() const
{
    if (!is_null())
    {
        char* s = 0;
        Dwarf_Error err = 0;

        switch (this->form())
        {
        case DW_FORM_strp:
        case DW_FORM_string:
            if (dwarf_formstring(attr(), &s, &err) == DW_DLV_ERROR)
            {
                THROW_ERROR(dbg(), err);
            }
 
            return s;

            //dwarf_dealloc(dbg(), s, DW_DLA_STRING);
            // quote from libdwarf's source:
            /* Contrary to long standing documentation,
             * The string pointer returned thru return_str must
             *   never have dwarf_dealloc() applied to it.
             *   Documentation fixed July 2005.
             */
            break;

        default:
            LOG_ERROR << "CompDirAttr::value(): unhandled form 0x"
                      << hex << form() << dec << endl;

            break;
        }
    }

    return "";
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
