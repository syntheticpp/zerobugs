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

#include <assert.h>
#include <stdexcept>
#include "debug.h"
#include "error.h"
#include "utils.h"
#include "private/generic_attr.h"
#include "private/factory.h"
#include "private/indirect.h"
#include "impl.h"
#include "die.h"


using namespace std;


Dwarf::Die::Die(Dwarf_Debug dbg, Dwarf_Die die)
    : dbg_(dbg)
    , die_(die)
    , name_(0)
{
    assert(die_);
}


Dwarf::Die::~Die() throw()
{
    if (name_)
    {
        free(name_);
    }
    dwarf_dealloc(dbg(), die_, DW_DLA_DIE);
}


const char* Dwarf::Die::name() const
{
    if (name_ == 0)
    {
        char* name = name_impl();
        if (!name)
        {
            name = strdup("");
        }
        if (!name)
        {
            throw std::bad_alloc();
        }
        name_ = name;
    }
    return name_ ? name_ : "";
}


void Dwarf::Die::set_name(const char* name)
{
    char* tmp = strdup(name);
    if (!tmp)
    {
        throw std::bad_alloc();
    }
    free(name_);
    name_ = tmp;
}


string Dwarf::Die::name(Dwarf_Debug dbg, Dwarf_Die die)
{
    string result;

    Dwarf_Error err;
    char* name = 0;
    try
    {
        if (dwarf_diename(die, &name, &err) == DW_DLV_ERROR)
        {
            dwarf_dealloc(dbg, err, DW_DLA_ERROR);
        }
        else
        {
            result = name;
        }
    }
    catch (...)
    {
    }
    dwarf_dealloc(dbg, name, DW_DLA_STRING);
    return result;
}


char* Dwarf::Die::name_impl() const
{
    Dwarf_Error err = 0;
    char* name = 0;

    if (dwarf_diename(die_, &name, &err) == DW_DLV_ERROR)
    {
        THROW_ERROR(dbg(), err);
    }
    char* result = (name && *name) ? strdup(name) : 0;
    dwarf_dealloc(dbg(), name, DW_DLA_STRING);
    return result;
}


Dwarf_Half Dwarf::Die::get_tag() const
{
    return Utils::tag(this->dbg(), this->die());
}


Dwarf_Off Dwarf::Die::offset(Dwarf_Debug dbg, Dwarf_Die die)
{
    Dwarf_Error err = 0;
    Dwarf_Off off = 0;

    if (dwarf_dieoffset(die, &off, &err) == DW_DLV_ERROR)
    {
        THROW_ERROR(dbg, err);
    }
    return off;
}


Dwarf_Off Dwarf::Die::cu_offset() const
{
    Dwarf_Error err = 0;
    Dwarf_Off off = 0;

    if (dwarf_die_CU_offset(die_, &off, &err) == DW_DLV_ERROR)
    {
        THROW_ERROR(dbg(), err);
    }
    return off;
}


const Dwarf::Debug& Dwarf::Die::owner() const
{
    Debug* owner = Debug::get_wrapper(dbg());
    if (!owner)
    {
        // no die should survive (snort) the Debug
        // wrapper that owns it
        throw logic_error("Null Dwarf::Die::owner");
    }
    return *owner;

}


Dwarf::Debug& Dwarf::Die::owner()
{
    Debug* owner = Debug::get_wrapper(dbg());
    if (!owner)
    {
        throw logic_error("Null Dwarf::Die::owner");
    }
    return *owner;
}



boost::shared_ptr<Dwarf::Die>
Dwarf::Die::check_indirect(bool useSpec) const
{
#if 1
    if (!indirect_)
    {
        indirect_ = get_indirect<DW_AT_abstract_origin>(dbg(), die());
    }
    if (!indirect_ && useSpec)
    {
        indirect_ = get_indirect<DW_AT_specification>(dbg(), die());
    }
    return indirect_;
#else
    boost::shared_ptr<Die> indirect = get_indirect<DW_AT_abstract_origin>(dbg(), die());
    if (!indirect && useSpec)
    {
        indirect = get_indirect<DW_AT_specification>(dbg(), die());
    }
    return indirect;
#endif
}


boost::shared_ptr<Dwarf::Die> Dwarf::Die::import() const
{
    boost::shared_ptr<Die> imp(get_indirect<DW_AT_import>(dbg(), die()));
    if (!imp)
    {
        if (boost::shared_ptr<Die> indirect = check_indirect())
        {
            imp = get_indirect<DW_AT_import>(dbg(), indirect->die());
        }
    }
    return imp;
}


bool Dwarf::Die::is_artificial() const
{
    bool result = false;

    if (Utils::has_attr(dbg(), die(), DW_AT_artificial))
    {
        GenericAttr<DW_AT_artificial, Dwarf_Bool> art(dbg(), die());
        result = art.value();
    }
    return result;
}


Dwarf::Access Dwarf::Die::access() const
{
    if (!Utils::has_attr(dbg(), die(), DW_AT_accessibility))
    {
        return a_public;
    }
    GenericAttr<DW_AT_accessibility, Dwarf_Unsigned> attr(dbg(), die());
    return static_cast<Access>(attr.value());
}


size_t Dwarf::Die::decl_line() const
{
    size_t lineNum = 0;

    if (Utils::has_attr(dbg(), die(), DW_AT_decl_line))
    {
        GenericAttr<DW_AT_decl_line, Dwarf_Unsigned> attr(dbg(), die());
        lineNum = attr.value();
    }
    return lineNum;
}


size_t Dwarf::Die::decl_file() const
{
    size_t fileIndex = 0;

    if (Utils::has_attr(dbg(), die(), DW_AT_decl_file))
    {
        GenericAttr<DW_AT_decl_file, Dwarf_Unsigned> attr(dbg(), die());
        fileIndex = attr.value();
    }
    return fileIndex;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
