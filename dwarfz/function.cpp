//
// $Id: function.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/shared_string_impl.h"
#include <algorithm>
#include <cassert>
#include "compile_unit.h"
#include "debug.h"
#include "location.h"
#include "parameter.h"
#include "type.h"
#include "utils.h"
#include "private/factory.h"
#include "private/generic_attr.h"
#include "private/indirect.h"
#include "private/log.h"
#include "impl.h"
#include "function.h"

using namespace std;
using namespace boost;
using namespace Dwarf;


Dwarf::Function::Function (
    Dwarf_Debug dbg,
    Dwarf_Die   die,
    Dwarf_Addr  lowPC,
    Dwarf_Addr  highPC
    )
    : Block(dbg, die)
    , unit_(0) // set by the CompileUnit
    , isMemFun_(false)
    , lowPC_(lowPC)
    , highPC_(highPC)
{
    assert(Utils::tag(dbg, die) == DW_TAG_subprogram);
}


const Function::ParamList&
Function::params() const
{
    cache_params();
    assert(params_.get());
    return *params_;
}


shared_ptr<Type>
Dwarf::Function::ret_type() const
{
    return Utils::type(*this);
}


char*
Dwarf::Function::name_impl() const
{
    char* name = Die::name_impl();
    if (!name)
    {
        if (shared_ptr<Die> indirect = check_indirect())
        {
            name = strdup(indirect->name());
        }
    }
    return name;
}


RefPtr<SharedString>
Dwarf::Function::linkage_name() const
{
    if (linkageName_.is_null())
    {
        if (Utils::has_attr(dbg(), die(), DW_AT_MIPS_linkage_name))
        {
            GenericAttr<DW_AT_MIPS_linkage_name, char*> attr(dbg(), die());
            linkageName_ = shared_string(attr.str());
            if (RefPtr<StringCache> cache = owner().string_cache().lock())
            {
                linkageName_ = cache->get_string(linkageName_.get());
            }
        }
    }
    return linkageName_;
}


const Dwarf::TypeList&
Dwarf::Function::param_types() const
{
    cache_params();
    assert(paramTypes_.get());
    return *paramTypes_;
}


void
Dwarf::Function::cache_params() const
{
    if ((params_.get() == 0) || (paramTypes_.get() == 0))
    {
        params_.reset(new ParamList);
        paramTypes_.reset(new TypeList);

        typedef List<ParameterT<> > ArgList;

        ArgList paramList(this->dbg(), this->die());

        ArgList::const_iterator i = paramList.begin();
        const ArgList::const_iterator end = paramList.end();

        for (; i != end; ++i)
        {
            if (i->is_artificial())
            {
                // the `this' parameter is artificial for
                // C++ methods; it is also expected to be
                // the first parameter
                // todo: any other valid cases for artificial params?
                //
                isMemFun_ = true;
            }
            if (shared_ptr<Type> type = i->type())
            {
                paramTypes_->push_back(type);
                params_->push_back(i);
            }
            else
            {
                cerr << name() << ": Unknown type for parameter " << i->name() << endl;
            }
        }
    }
}


bool
Dwarf::Function::compare_prototype(const Function& other) const
{
    if (has_variable_args() != other.has_variable_args())
    {
        return false;
    }
    if (ret_type())
    {
        if (!other.ret_type())
        {
            return false;
        }
        if (ret_type()->offset() != other.ret_type()->offset())
        {
            return false;
        }
    }
    else if (other.ret_type())
    {
        return false;
    }
    if (param_types().size() != other.param_types().size())
    {
        return false;
    }
    TypeList::const_iterator i = param_types().begin();
    TypeList::const_iterator j = other.param_types().begin();

    if (isMemFun_ || other.isMemFun_)
    {
        // first param is artificial, skip it
        assert(!param_types().empty());
        ++i, ++j;
    }
    for (; i != param_types().end(); ++i, ++j)
    {
        assert(j != other.param_types().end());
        assert(*i);
        assert(*j);

        if ((*i)->offset() != (*j)->offset())
        {
            return false;
        }
    }
    return true;
}


bool
Dwarf::Function::has_variable_args() const
{
    return Utils::has_child(*this, DW_TAG_unspecified_parameters);
}


Dwarf_Addr
Dwarf::Function::low_pc() const
{
    if (lowPC_ == 0)
    {
        lowPC_ = Block::low_pc();
    }
    return lowPC_;
}


Dwarf_Addr
Dwarf::Function::high_pc() const
{
    if (highPC_ == 0)
    {
        highPC_ = Block::high_pc();
        log<debug>(2) << __func__ << "=" << hex << highPC_ << dec << "\n";
    }
    return highPC_;
}


Dwarf_Addr
Dwarf::Function::frame_base( Dwarf_Addr moduleBase,
                             Dwarf_Addr frameBase,
                             Dwarf_Addr pc
                           ) const
{
    const Dwarf_Addr base = unit() ? unit()->base_pc() : 0;
    Dwarf_Addr result = frameBase;
    if (boost::shared_ptr<Location> loc =
        Utils::loc(dbg(), die(), DW_AT_frame_base))
    {
        result = loc->eval(frameBase, moduleBase, base, pc);
    }
    return result;
}


bool
Dwarf::Function::inline_not_inlined() const
{
    if (Utils::has_attr(dbg(), die(), DW_AT_inline))
    {
        GenericAttr<DW_AT_inline, Dwarf_Unsigned> attr(dbg(), die());
        if (attr.value() == DW_INL_declared_not_inlined)
        {
            return true;
        }
    }
    if (shared_ptr<Die> tmp = check_indirect())
    {
        if (shared_ptr<Function> fun = shared_dynamic_cast<Function>(tmp))
        {
            if (fun->inline_not_inlined())
            {
                return true;
            }
        }
    }
    return false;
}


Dwarf::CallingConvention
Dwarf::Function::calling_convention() const
{
    CallingConvention result = CC_normal;

    if (Utils::has_attr(dbg(), die(), DW_AT_calling_convention))
    {
        GenericAttr<DW_AT_calling_convention, Dwarf_Unsigned> cc(dbg(), die());
        result = static_cast<CallingConvention>(cc.value());
    }
    return result;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
