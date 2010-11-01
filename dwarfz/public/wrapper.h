#ifndef WRAPPER_H__E45E60E8_679F_465E_9DAE_594175807950
#define WRAPPER_H__E45E60E8_679F_465E_9DAE_594175807950
//
// $Id: wrapper.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <stdexcept>

#include "die.h"
#include "error.h"
#include "interface.h"



namespace Dwarf
{
class CompileUnit;

template<typename T>
struct WrapperTraits
{
    static int name(T, char**, Dwarf_Error*);
    static void dealloc(Dwarf_Debug, T);
    static int die_offset(T, Dwarf_Off*, Dwarf_Error*);
    static int cu_offset(T, Dwarf_Off*, Dwarf_Error*);
    static int objects(Dwarf_Debug, T**, Dwarf_Signed*, Dwarf_Error*);
};


template<typename T> CLASS Wrapper
{
    typedef WrapperTraits<T> traits;

    // non-copyable, non-assignable
    Wrapper(const Wrapper&);
    Wrapper& operator=(const Wrapper&);

public:
    Wrapper(Dwarf_Debug dbg, T obj) : dbg_(dbg), obj_(obj), name_(0)
    {
    }


    ~Wrapper() throw()
    {
        assert(dbg_);
        // wrapper is currently used for Dwarf_Var and Dwarf_Global;
        // in both cases, the name returned by dwarf_globname and
        // dwarf_varname, respectively, is a pointer to the internal
        // structure, no dealloc needed
        /* if (name_)
        {
            dwarf_dealloc(dbg_, name_, DW_DLA_STRING);
        } */
        if (obj_)
        {
            traits::dealloc(dbg_, obj_);
        }
    }


    /**
     * The name of the die referred to by this descriptor;
     * callers must NOT attempt to free the returned string
     */
    const char* name() const
    {
        if (name_ == 0)
        {
            Dwarf_Error err = 0;
            if (traits::name(obj_, &name_, &err) == DW_DLV_ERROR)
            {
                throw Error(__func__, dbg_, err);
            }
        }
        return name_;
    }


    /* get the Dwarf_Die */
    /* boost::shared_ptr<Die> get_die() const
    {
        Dwarf_Error err = 0;
        Dwarf_Off   off = 0;

        if (traits::die_offset(obj_, &off, &err) == DW_DLV_ERROR)
        {
            throw Error(__func__, err);
        }
        if (Debug* dptr = Debug::get_debug(dbg_))
        {
            return dptr->get_object(off);
        }
        throw std::runtime_error("get_die: invalid handle");
    } */


    Dwarf_Off die_offset() const
    {
        Dwarf_Error err = 0;
        Dwarf_Off   off = 0;

        if (traits::die_offset(obj_, &off, &err) == DW_DLV_ERROR)
        {
            throw Error(__func__, dbg_, err);
        }
        return off;
    }


    Dwarf_Off cu_offset() const
    {
        Dwarf_Error err = 0;
        Dwarf_Off   off = 0;

        if (traits::cu_offset(obj_, &off, &err) == DW_DLV_ERROR)
        {
            throw Error(__func__, err);
        }
        return off;
    }

private:
    Dwarf_Debug     dbg_;
    T               obj_;
    mutable char*   name_;
};
} // namespace Dwarf

#endif // WRAPPER_H__E45E60E8_679F_465E_9DAE_594175807950
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
