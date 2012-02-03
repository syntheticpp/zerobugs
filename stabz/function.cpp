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
//
#include <iostream>
#include <stdexcept>
#include <sstream>
#include "dharma/environ.h"
#include "stabz/public/function.h"
#include "stabz/public/variable.h"



using namespace std;


Stab::Function::~Function() throw()
{
}


Stab::Function::Function
(
    addr_t addr,
    size_t index,
    SharedString& name
)
: Stab::Block(addr, index)
, name_(&name)
{
}


SharedString& Stab::Function::name() const
{
    assert(!name_.is_null());
    return *name_;
}


RefPtr<DataType> Stab::Function::return_type() const
{
    return returnType_.ref_ptr();
}


void Stab::Function::set_return_type(const RefPtr<DataType>& type)
{
#ifdef DEBUG
    if (returnType_.ref_ptr() && env::get("ZERO_ABORT_ON_ERROR", 0) == 1)
    {
        abort();
    }
#endif
    assert(type.is_null() || type->ref_count() > 0);
    returnType_ = type;
}


#ifdef DEBUG
static void dump_vars(ostream& outs, const Stab::Block::VarList& vars)
{
    Stab::Block::VarList::const_iterator i = vars.begin();
    for (; i != vars.end(); ++i)
    {
        outs << (*i)->name().c_str() << endl;
    }
}
#endif


void Stab::Function::assign_variables(const VarList& vars)
{
    if (!variables().empty())
    {
#ifdef DEBUG
        ostream& err = cerr;
        err << "*** Warning: ";
        err << name().c_str() << " index=";
        err << begin_index()  << " already has ";
        err << variables().size() << " variable(s).\n";

        err << "Attempting to ";

        if (vars == variables())
        {
            err << "re-assign variables";
        }
        else
        {
            err << "assign different variables";
        }
        err << endl;
        err << "=== current variables: ===\n";
        dump_vars(err, variables());
        err << "\n=== new variables: ===\n";
        dump_vars(err, vars);
#endif
    }
    else
    {
        Block::assign_variables(vars);
    }
}


static void
collect_params(const Stab::Block::VarList& vlist, ParamTypes& paramTypes)
{
    using namespace Stab;

    Block::VarList::const_iterator i = vlist.begin();
    const Block::VarList::const_iterator end = vlist.end();

    for (; i != end; ++i)
    {
        if (Parameter* param = interface_cast<Parameter*>(i->get()))
        {
            paramTypes.push_back(param->type());
        }
    }
}


const ParamTypes& Stab::Function::param_types() const
{
    if (!paramTypes_.get())
    {
        paramTypes_.reset(new ParamTypes);

        collect_params(variables(), *paramTypes_);

        if (!blocks().empty())
        {
            collect_params(blocks().front()->variables(), *paramTypes_);
        }
    }
    return *paramTypes_;
}


const Stab::Block::VarList& Stab::Function::params() const
{
    if (!params_.get())
    {
        params_.reset(new VarList);

        VarList::const_iterator i = variables().begin();
        for (; i != variables().end(); ++i)
        {
            if (interface_cast<Parameter*>(i->get()))
            {
                params_->push_back(*i);
            }
        }
    }
    return *params_;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
