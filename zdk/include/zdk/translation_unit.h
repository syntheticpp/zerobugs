#ifndef UNIT_H__A7F13D4A_8183_455B_9739_BEB0D033A09F
#define UNIT_H__A7F13D4A_8183_455B_9739_BEB0D033A09F
//
// $Id: translation_unit.h 714 2010-10-17 10:03:52Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/enum.h"
#include "zdk/platform.h"
#include "zdk/ref_counted.h"
#include "zdk/shared_string.h"

using Platform::addr_t;

/**
 * Models a translation unit (aka compilation unit).
 */
DECLARE_ZDK_INTERFACE_(TranslationUnit, ZObject)
{
    DECLARE_UUID("a6b96b39-9011-49cf-889e-02ac45c00a2f")

    virtual const char* filename() const = 0;

    virtual Platform::addr_t lower_addr() const = 0;

    virtual Platform::addr_t upper_addr() const = 0;

    /**
     * Enumerate namespaces contained in this unit
     * @param name if not NULL, return non-zero if the
     * unit contains the namespace, or zero
     * @param cb callback object, if not NULL then its
     * notify method is called for each enumerated namespace.
     * @return number of namespaces enumerated
     */
    virtual size_t enum_ns(const char* name,
                           EnumCallback<const char*>* cb
                          ) const = 0;

    virtual const char* producer() const = 0;

    /**
     * @return the id of the programming language this
     * unit was written in; the values are as encoded
     * in dwarf.h
     */
    virtual int language() const = 0;

    virtual size_t enum_sources(EnumCallback<SharedString*, bool>*) = 0;

    /**
     * @return the name of the module this compilation unit is part of
     * @note Here module is taken in the sense of a Shared Object or
     * executable.
     * C++ does not have language support for modules
     */
    virtual SharedString* module_name() const = 0;

    /**
     * @return the name of the module as specified in the source code,
     * for languages that have explicit module support (such as D)
     */
    virtual SharedString* declared_module() const = 0;

    /**
     * @see DebugInfoReader::addr_to_line
     */
    /*
    virtual size_t addr_to_line(
        addr_t              addr,
        addr_t*             nearest,
        EnumCallback2<SharedString*, size_t>* cb) = 0;

    virtual size_t line_to_addr(size_t, EnumCallback<addr_t>*) = 0;

    */

    ///@todo: more methods here, enumerate funcs, etc.
};


#endif // UNIT_H__A7F13D4A_8183_455B_9739_BEB0D033A09F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
