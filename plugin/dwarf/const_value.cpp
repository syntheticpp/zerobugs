//
// $Id: const_value.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <sstream>
#include <boost/shared_ptr.hpp>
#include "const_value.h"
#include "dwarfz/public/const_value.h"
#include "dwarfz/public/datum.h"
#include "typez/public/debug_symbol_impl.h"
#include "zdk/types.h"

using namespace boost;
using namespace std;


RefPtr<DebugSymbolImpl>
Dwarf::const_value( Thread& thread,
                    const Dwarf::Datum& datum,
                    DataType& type,
                    SharedString* name)
{
    RefPtr<DebugSymbolImpl> value;
    if (shared_ptr<ConstValue> constVal = datum.const_value())
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
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
