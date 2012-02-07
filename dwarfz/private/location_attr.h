#ifndef LOCATION_ATTR_H__249749DE_9D0F_407D_B63D_DCB011EBEE28
#define LOCATION_ATTR_H__249749DE_9D0F_407D_B63D_DCB011EBEE28
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
#include <memory>
#include "attr.h"
#include "interface.h"
#include "location.h"

namespace Dwarf
{
    class Location;

    /**
     * Base for location attributes, see LocationAttrT
     */
    CLASS LocationAttr : public Attribute
    {
    public:
        typedef std::shared_ptr<Location> loc_ptr_type;
        
        LocationAttr(Dwarf_Debug, Dwarf_Die, Dwarf_Half);

        virtual loc_ptr_type value() const;

        virtual int kind() const = 0;

        virtual ~LocationAttr() throw (){}

        static std::shared_ptr<LocationAttr>
        create_instance(Dwarf_Debug, Dwarf_Die, Dwarf_Half);
    };


    /**
     * Wrapper for:
     * DW_AT_location attributes,
     * DW_AT_frame_base,
     * DW_AT_data_member_location
     * DW_AT_vtable_elem_location
     */
    template<Dwarf_Half AT>
    CLASS LocationAttrT : public LocationAttr
    {
    public:
        LocationAttrT(Dwarf_Debug dbg, Dwarf_Die die)
            : LocationAttr(dbg, die, AT)
        {}

    protected:
        int kind() const { return AT; }
    };


    template<>
    CLASS LocationAttrT<DW_AT_vtable_elem_location>
        : public LocationAttr
    {
    public:
        LocationAttrT(Dwarf_Debug dbg, Dwarf_Die die)
            : LocationAttr(dbg, die, DW_AT_vtable_elem_location)
        {}

    protected:
        int kind() const { return DW_AT_vtable_elem_location; }

        virtual loc_ptr_type value() const
        {
            return std::make_shared<VTableElemLocation>(this->dbg(), this->attr());
        }
    };
} // namespace Dwarf

#endif // LOCATION_ATTR_H__249749DE_9D0F_407D_B63D_DCB011EBEE28
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

