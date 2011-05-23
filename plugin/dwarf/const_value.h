#ifndef CONST_VALUE_H__D247FABC_08C0_437B_83B1_7F72F61498B8
#define CONST_VALUE_H__D247FABC_08C0_437B_83B1_7F72F61498B8
//
// $Id: const_value.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/ref_ptr.h"
#include <sstream>
#include <boost/shared_ptr.hpp>
#include "const_value.h"
#include "dwarfz/public/const_value.h"
#include "dwarfz/public/datum.h"
#include "typez/public/debug_symbol_impl.h"
#include "zdk/types.h"

class DataType;
class DebugSymbolImpl;
class SharedString;
class Thread;


namespace Dwarf
{
    using namespace boost;
    using namespace std;


    /**
     * @return a debug symbol that corresponds to the
     * DW_AT_const_value attribute of the given datum, or NULL
     */
    template<typename T>
    RefPtr<DebugSymbolImpl>
    const_value( Thread& thread,
                 const T& datum,
                 DataType& type,
                 SharedString* name)
    {
        RefPtr<DebugSymbolImpl> value;
        if (boost::shared_ptr<ConstValue> constVal = datum.const_value())
        {
            ostringstream buf;
            if (interface_cast<FloatType*>(&type))
            {
                switch(constVal->size())
                {
                case sizeof(float):
                    buf << *static_cast<const float*>(constVal->data());
                    break;
                case sizeof(double):
                    buf << *static_cast<const double*>(constVal->data());
                    break;
                case sizeof(long double):
                    buf << *static_cast<const long double*>(constVal->data());
                    break;
                }
            }
            else
            {
                assert(constVal->size() >= sizeof(Dwarf_Unsigned));
                buf << *static_cast<const Dwarf_Unsigned*>(constVal->data());
            }
            //string tmp(klass_->name()->c_str());
            //tmp += "::";
            //tmp += name->c_str();
            value = DebugSymbolImpl::create(thread,
                                            type,
                                            buf.str(),
                                            //shared_string(tmp).get());
                                            name);
            value->set_constant();
        }
        return value;
    }
}

#endif // CONST_VALUE_H__D247FABC_08C0_437B_83B1_7F72F61498B8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
