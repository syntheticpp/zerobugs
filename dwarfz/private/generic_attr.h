#ifndef GENERIC_ATTR_H__5D0B31B3_52AD_4337_81EF_6DCD2AA4ED7B
#define GENERIC_ATTR_H__5D0B31B3_52AD_4337_81EF_6DCD2AA4ED7B
//
// $Id: generic_attr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <dwarf.h>
#include <string.h>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include "attr.h"
#include "error.h"
#include "interface.h"
#include "utils.h"
#include "log.h"


namespace Dwarf
{
    using namespace std;


    template<Dwarf_Half A, typename T>
    CLASS GenericAttr : public Attribute
    {
    public:
        typedef T value_type;

        GenericAttr(Dwarf_Debug dbg, Dwarf_Die die)
            : Attribute(dbg, die, A)
        { }

        value_type value() const
        {
            value_type v = value_type();
            union
            {
                Dwarf_Unsigned u;
                Dwarf_Signed s;
                Dwarf_Bool b;
                Dwarf_Addr a;
                // char* str;
            } tmp = { 0 };

            Dwarf_Error err = 0;

            const Dwarf_Half f = this->form();
            switch (f)
            {
            case DW_FORM_addr:
                if (dwarf_formaddr(attr(), &tmp.a, &err) == DW_DLV_ERROR)
                {
                    throw Error("dwarf_formaddr", dbg(), err);
                }
                v = tmp.a;
                break;

            case DW_FORM_flag:
                if (dwarf_formflag(attr(), &tmp.b, &err) == DW_DLV_ERROR)
                {
                    throw Error("dwarf_formflag", dbg(), err);
                }
                v = tmp.b;
                break;

            case DW_FORM_data1:
            case DW_FORM_data2:
            case DW_FORM_data4:
            case DW_FORM_data8:
            case DW_FORM_udata:
                if (dwarf_formudata(attr(), &tmp.u, &err) == DW_DLV_ERROR)
                {
                    throw Error("dwarf_formudata", dbg(), err);
                }
                v = tmp.u;
                break;

            case DW_FORM_ref1:
            case DW_FORM_ref2:
            case DW_FORM_ref4:
            case DW_FORM_ref8:
            case DW_FORM_ref_addr:
                if (dwarf_global_formref(attr(), &tmp.u, &err) == DW_DLV_ERROR)
                {
                    throw Error("dwarf_global_formref", dbg(), err);
                }
                v = tmp.u;
                break;

            case DW_FORM_sdata:
                if (dwarf_formsdata(attr(), &tmp.s, &err) == DW_DLV_ERROR)
                {
                    throw Error("dwarf_formsdata", dbg(), err);
                }
                v = tmp.s;
                break;

    /* for T == char* use str()
            case DW_FORM_strp:
            case DW_FORM_string:
     */
            default:
                log<warn>() << "GenericAttr<0x" << hex << A << ">\n"
                            << typeid(*this).name()
                            << ": unhandled form=0x"
                            << hex << f << dec << "\n";
            }
            return v;
        }


        string str() const
        {
            char* s = 0;
            Dwarf_Error err = 0;

            switch(this->form())
            {
            case DW_FORM_strp:
            case DW_FORM_string:
                if (dwarf_formstring(this->attr(), &s, &err) == DW_DLV_ERROR)
                {
                    throw Error("dwarf_formstring", dbg(), err);
                }
                break;

            default:
                log<warn>() << "form=" << hex << this->form() << dec << "\n";
                throw logic_error("str() invoked with incorrect attribute form");
            }

            string result;
            if (s)
            {
                result = s;
                dwarf_dealloc(dbg(), s, DW_DLA_STRING);
            }
            return result;
        }


        vector<char> block() const
        {
            Dwarf_Block* blkp = NULL;
            Dwarf_Error err = 0;
            vector<char> data;
            switch (this->form())
            {
            case DW_FORM_block:
            case DW_FORM_block1:
            case DW_FORM_block2:
            case DW_FORM_block4:
                if (dwarf_formblock(this->attr(), &blkp, &err) == DW_DLV_ERROR)
                {
                    throw Error("dwarf_formblock", dbg(), err);
                }
                if (blkp)
                {
                    data.resize(blkp->bl_len);
                    memcpy(&data[0], blkp->bl_data, blkp->bl_len);
                }
                break;

            //default:
            //    throw logic_error("block() invoked with incorrect attribute form");
            }
            return data;
        }
    };
}

#endif // GENERIC_ATTR_H__5D0B31B3_52AD_4337_81EF_6DCD2AA4ED7B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
