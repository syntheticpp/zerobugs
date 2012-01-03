//
// $Id: utils.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <dwarf.h>
#include <string>
#include "error.h"
#include "debug.h" // for the incomplete type hack
#include "function.h"
#include "generic/temporary.h"
#include "location_attr.h"
#include "type.h"
#include "type_attr.h"
#include "private/generic_attr.h"
#include "private/log.h"
#include "impl.h"
#include "utils.h"

#ifdef __func__
#undef __func__
#endif

using namespace std;
using namespace Dwarf;


static bool tag_in_list
(
    Dwarf_Half          tag,
    const Dwarf_Half*   tags,
    size_t              ntags
)
{
    assert(tags);
    for (size_t i = 0; i != ntags; ++i)
    {
        if (tags[i] == tag)
        {
            return true;
        }
    }
    return false;
}


Dwarf_Die Utils::first_child
(
    Dwarf_Debug         dbg,
    Dwarf_Die           die,
    const Dwarf_Half*   tags,
    size_t              ntags,
    Dwarf_Half*         retTag
)
{
    Dwarf_Error err = 0;
    Dwarf_Die child = 0;

    if (dwarf_child(die, &child, &err) == DW_DLV_ERROR)
    {
        THROW_ERROR(dbg, err);
    }
    if (child)
    {
        Dwarf_Half t = tag(dbg, child);
        if (tag_in_list(t, tags, ntags))
        {
            if (retTag)
            {
                *retTag = t;
            }
            return child;
        }
        else
        {
            return next_sibling(dbg, child, tags, ntags, retTag);
        }
    }
    return NULL;
}


Dwarf_Die Utils::next_sibling
(
    Dwarf_Debug         dbg,
    Dwarf_Die           die,
    const Dwarf_Half*   tags,
    size_t              ntags,
    Dwarf_Half*         retTag
)
{
    for (Dwarf_Die prev = die; prev;)
    {
        Dwarf_Error error = 0;
        Dwarf_Die   entry = 0;

        int rc = dwarf_siblingof(dbg, prev, &entry, &error);

        if (rc == DW_DLV_ERROR)
        {
            THROW_ERROR(dbg, error);
        }
        if (error)
        {
            dwarf_dealloc(dbg, error, DW_DLA_ERROR);
        }
        if (prev && prev != die)
        {
            dwarf_dealloc(dbg, prev, DW_DLA_DIE);
        }
        if (rc != DW_DLV_NO_ENTRY)
        {
            assert(entry);
            Dwarf_Half t = tag(dbg, entry);
            if (tag_in_list(t, tags, ntags))
            {
                if (retTag)
                {
                    *retTag = t;
                }
                return entry;
            }
        }
        prev = entry;
    }
    return NULL;
}


bool Utils::has_child(const Dwarf::Die& die, Dwarf_Half tag)
{
    Dwarf_Die tmp = first_child(die.dbg(), die.die(), tag);
    bool result = tmp != 0;

    dwarf_dealloc(die.dbg(), tmp, DW_DLA_DIE);

    return result;
}


/**
 * Get the first child of DIE of the specified TAG_TYPE
 */
Dwarf_Die
Utils::first_child(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half tag)
{
    return first_child(dbg, die, &tag, 1, NULL);
}


/**
 * Get the next sibling of die, of the specified tag_type
 */
Dwarf_Die
Utils::next_sibling(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half tag)
{
    return next_sibling(dbg, die, &tag, 1, NULL);
}


/**
 * Get the tag of a given die
 */
Dwarf_Half Utils::tag(Dwarf_Debug dbg, Dwarf_Die die)
{
    Dwarf_Half tag = 0;
    Dwarf_Error err = 0;

    if (dwarf_tag(die, &tag, &err) == DW_DLV_ERROR)
    {
        THROW_ERROR(dbg, err);
    }
    return tag;
}


Dwarf_Half Utils::tag(Dwarf_Debug dbg, Dwarf_Die die, std::nothrow_t) throw()
{
    Dwarf_Half tag = 0;
    Dwarf_Error err = 0;

    if (dwarf_tag(die, &tag, &err) == DW_DLV_ERROR)
    {
        dwarf_dealloc(dbg, err, DW_DLA_ERROR);
        tag = 0;
    }
    return tag;
}


boost::shared_ptr<Location>
Utils::loc(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attr_type)
{
    boost::shared_ptr<Location> loc;
    boost::shared_ptr<LocationAttr> attr =
        LocationAttr::create_instance(dbg, die, attr_type);

    if (attr)
    {
        loc = attr->value();
    }
    return loc;
}


boost::shared_ptr<Location> Utils::loc(Dwarf_Debug dbg, Dwarf_Die die)
{
    return loc(dbg, die, DW_AT_location);
}


typedef boost::shared_ptr<Type> TypePtr; // short-hand


TypePtr Utils::type(const Dwarf::Die& die)
{
    TypePtr result = Utils::type(die.dbg(), die.die());
    if (!result)
    {
        boost::shared_ptr<Dwarf::Die> indirect = die.check_indirect();
        if (indirect)
        {
            result = Utils::type(*indirect);
        }
    }
    return result;
}


TypePtr Utils::type(Dwarf_Debug dbg, Dwarf_Die die)
{
    TypePtr type;
    TypeAttr attr(dbg, die);
    if (!attr.is_null())
    {
        type = attr.value();
    }
    return type;
}


TypePtr Utils::containing_type(Dwarf_Debug dbg, Dwarf_Die die)
{
    TypePtr type;
    TypeAttr attr(dbg, die, DW_AT_containing_type);
    if (!attr.is_null())
    {
        type = attr.value();
    }
    return type;
}


bool Utils::has_attr(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attr)
{
    Dwarf_Error err = 0;
    Dwarf_Bool  result = false;

    if (dwarf_hasattr(die, attr, &result, &err) == DW_DLV_ERROR)
    {
        THROW_ERROR(dbg, err);
    }
    return result;
}


Dwarf_Unsigned Utils::bit_size(Dwarf_Debug dbg, Dwarf_Die die)
{
    Dwarf_Unsigned size = 0;
    if (has_attr(dbg, die, DW_AT_bit_size))
    {
        Dwarf_Error err = 0;
        if (dwarf_bitsize(die, &size, &err) == DW_DLV_ERROR)
        {
            THROW_ERROR(dbg, err);
        }
    }
    else
    {
        size = byte_size(dbg, die) << 3;
    }
    return size;
}


Dwarf_Off Utils::bit_offset(Dwarf_Debug dbg, Dwarf_Die die)
{
    Dwarf_Off off = 0;

    if (has_attr(dbg, die, DW_AT_bit_offset))
    {
        Dwarf_Error err = 0;
        if (dwarf_bitoffset(die, &off, &err) == DW_DLV_ERROR)
        {
            THROW_ERROR(dbg, err);
        }
    }
    return off;
}


Dwarf_Unsigned Utils::byte_size(Dwarf_Debug dbg, Dwarf_Die die)
{
    Dwarf_Unsigned size = 0;

    if (has_attr(dbg, die, DW_AT_byte_size))
    {
        Dwarf_Error err = 0;
        if (dwarf_bytesize(die, &size, &err) == DW_DLV_ERROR)
        {
            THROW_ERROR(dbg, err);
        }
    }
    return size;
}


void Utils::dump_attributes(const Dwarf::Die& die, ostream& outs)
{
    outs << "----- " << die.name() << " ------\n";
    Dwarf_Attribute* atlist = 0;
    Dwarf_Signed atcnt = 0;
    Dwarf_Error err = 0;

    int atres = dwarf_attrlist(die.die(), &atlist, &atcnt, &err);
    if (atres == DW_DLV_ERROR)
    {
        THROW_ERROR(die.dbg(), err);
    }
    else if (atres == DW_DLV_NO_ENTRY)
    {
        // indicates there are no attrs.  It is not an error.
        atcnt = 0;
    }
    if (error)
    {
        dwarf_dealloc(die.dbg(), err, DW_DLA_ERROR);
    }

    for (int i = 0; i < atcnt; ++i)
    {
        Dwarf_Half attr;
        int ares = dwarf_whatattr(atlist[i], &attr, &err);
        if (ares == DW_DLV_OK)
        {
            outs << hex << "attr=" << attr << dec << endl;
        }
        else
        {
            THROW_ERROR(die.dbg(), err);
        }
        if (attr == DW_AT_accessibility)
        {
            GenericAttr<DW_AT_accessibility, Dwarf_Unsigned> a(die.dbg(), die.die());
            clog << "accessibility=" << a.value() << endl;
        }
    }
    for (int i = 0; i < atcnt; i++)
    {
        dwarf_dealloc(die.dbg(), atlist[i], DW_DLA_ATTR);
    }
    dwarf_dealloc(die.dbg(), atlist, DW_DLA_LIST);
}


void Utils::dump_children(const Dwarf::Die& die, ostream& outs)
{
    Dwarf_Error err = 0;
    Dwarf_Die child = 0;

    if (dwarf_child(die.die(), &child, &err) != DW_DLV_ERROR)
    {
        outs << hex << tag(die.dbg(), child) << dec << endl;
        for (;;)
        {
            Dwarf_Die tmp = 0;
            int rc = dwarf_siblingof(die.dbg(), child, &tmp, &err);
            if (rc != DW_DLV_OK || tmp == 0)
            {
                break;
            }
            dwarf_dealloc(die.dbg(), child, DW_DLA_DIE);
            child = tmp;
            outs << hex << tag(die.dbg(), child) << dec << endl;
        }
        dwarf_dealloc(die.dbg(), child, DW_DLA_DIE);
    }
}


boost::shared_ptr<Function>
Utils::lookup_function(const FunList& funcs,
                       Dwarf_Addr addr,
                       const char* linkage)
{
    boost::shared_ptr<Function> result;

    FunList::const_iterator i = funcs.begin();
    const FunList::const_iterator end = funcs.end();

    // NOTE: this lookup is of linear complexity O(N)
    // but it shouldn't affect performance too much; after all,
    // it is expected to have tens or hundreds of functions in
    // one translation unit -- if you have thousands, you maybe
    // do not need a debugger anyway ;)
    for (; i != end; ++i)
    {
        if (addr)
        {
            if ((*i)->low_pc() <= addr && (*i)->high_pc() > addr)
            {
                result = *i;
                break;
            }
        }
        if (linkage && (*i)->linkage_name()
                    && (*i)->linkage_name()->is_equal(linkage))
        {
            result = *i;
            break;
        }
    }
    return result;
}

bool
Utils::get_linkage_name(Dwarf_Debug dbg, Dwarf_Die die, string& name)
{
    bool result = false;
    if (Utils::has_attr(dbg, die, DW_AT_MIPS_linkage_name))
    {
        GenericAttr<DW_AT_MIPS_linkage_name, char*> attr(dbg, die);
        name = attr.str();
        result = true;
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
