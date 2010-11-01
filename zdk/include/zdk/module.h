#ifndef MODULE_H__E0C2031B_EF33_4FBD_BCA6_6A40C61A392C
#define MODULE_H__E0C2031B_EF33_4FBD_BCA6_6A40C61A392C
//
// $Id: module.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/enum.h"
#include "zdk/platform.h"
#include "zdk/zobject.h"


struct SharedString;
struct SymbolTable;
struct TranslationUnit;


/**
 * A program can consist of several modules (executable and
 * shared libraries) loaded into memory. This class models
 * a "module", i.e. a shared object or the main executable.
 */
DECLARE_ZDK_INTERFACE_(Module, ZObject)
{
    DECLARE_UUID("03a1ea14-d088-4ea2-9511-7edc34219f15")

    /**
     * the module fullpath, in canonical form
     */
    virtual SharedString* name() const = 0;

    virtual time_t last_modified() const = 0;

    virtual off_t adjustment() const = 0;

    /**
     * @return the address where the module is
     * currently located in memory
     */
    virtual Platform::addr_t addr() const = 0;

    virtual Platform::addr_t upper() const = 0;

    /**
     * @return the head of a linked list of symbol tables
     * for this module
     */
    virtual SymbolTable* symbol_table_list() const = 0;

    /**
     * Enumerate the compilation units in this module
     */
    virtual size_t enum_units(EnumCallback<TranslationUnit*, bool>*) = 0;
};

#endif // MODULE_H__E0C2031B_EF33_4FBD_BCA6_6A40C61A392C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
