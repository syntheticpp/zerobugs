#ifndef COMP_DIR_ATTR_H__7F001BC8_C274_446E_90BE_53221C18D5F1
#define COMP_DIR_ATTR_H__7F001BC8_C274_446E_90BE_53221C18D5F1
//
// $Id: comp_dir_attr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <string>
#include "attr.h"
#include "interface.h"

namespace Dwarf
{
    /**
     * Wrapper for DW_AT_comp_dir
     */
    class CompDirAttr : public Attribute
    {
        friend class CompileUnit;

    public:
        /**
         * @return the compile directory
         */
        const char* value() const;

    protected:
        CompDirAttr(Dwarf_Debug, Dwarf_Die);
    };
}

#endif // COMP_DIR_ATTR_H__7F001BC8_C274_446E_90BE_53221C18D5F1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
