#ifndef REGISTER_H__60C77986_AA35_4B22_8C57_6B354C190804
#define REGISTER_H__60C77986_AA35_4B22_8C57_6B354C190804
//
// $Id: register.h 714 2010-10-17 10:03:52Z root $
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

struct Variant;


/**
 * Abstract CPU register
 */
DECLARE_ZDK_INTERFACE_(Register, ZObject)
{
    DECLARE_UUID("8a7276b7-0ffa-4bb6-97c5-1c64f32d64b7")

    typedef Platform::reg_t reg_t;

   /**
    * @return the name of this register
    */
    virtual const char* name() const = 0;

   /**
    * @return the size of this register in bytes
    */
    virtual size_t size() const = 0;

    virtual Variant* value() const = 0;

    /**
     * Modify the value stored inside of a register.
     * @param value the new value
     * @param name optional name, useful for register
     * with sub-fields, such as the x86 flags
     * @note The string parameter will be formatted
     * internally, according to the type of the register.
     */
    virtual bool
        set_value(const char* value, const char* name = 0) = 0;

    /**
     * Enumerate sub-fields if any, useful for flags registers.
     * The callback notify method will receive the name of the
     * field, its value, and a bitmask which indicates the
     * position and width of the field within the register.
     */
    virtual size_t enum_fields(
        EnumCallback3<const char*, reg_t, reg_t>*) const = 0;
};

#endif // REGISTER_H__60C77986_AA35_4B22_8C57_6B354C190804
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
