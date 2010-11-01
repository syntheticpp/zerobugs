#ifndef GLOBAL_H__E45E60E8_679F_465E_9DAE_594175807950
#define GLOBAL_H__E45E60E8_679F_465E_9DAE_594175807950
//
//
// $Id: global.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "generic/lock.h"
#include "zdk/mutex.h"
#include "wrapper.h"

namespace Dwarf
{
//#define LOCK_GLOBALS Lock<Mutex> lock(mutex);
#define LOCK_GLOBALS

/**
 * sPecialized traits for Dwarf_Global
 */
template<> struct WrapperTraits<Dwarf_Global>
{
    // static Mutex mutex;

    inline static int name(Dwarf_Global glob, char** name, Dwarf_Error* err)
    { LOCK_GLOBALS return dwarf_globname(glob, name, err); }

    inline static void dealloc(Dwarf_Debug dbg, Dwarf_Global glob)
    { LOCK_GLOBALS dwarf_dealloc(dbg, glob, DW_DLA_GLOBAL); }

    inline static int die_offset(Dwarf_Global glob, Dwarf_Off* off, Dwarf_Error* err)
    { LOCK_GLOBALS return dwarf_global_die_offset(glob, off, err); }

    inline static int cu_offset(Dwarf_Global glob, Dwarf_Off* off, Dwarf_Error* err)
    { LOCK_GLOBALS return dwarf_global_cu_offset(glob, off, err); }

    inline static int objects(
        Dwarf_Debug dbg,
        Dwarf_Global** glob,
        Dwarf_Signed* cnt,
        Dwarf_Error* err)
    {
        LOCK_GLOBALS
        return dwarf_get_globals(dbg, glob, cnt, err);
    }
};

typedef Wrapper<Dwarf_Global> Global;
}
#endif // GLOBAL_H__E45E60E8_679F_465E_9DAE_594175807950
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
