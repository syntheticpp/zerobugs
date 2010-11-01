//
// $Id: ident.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include "zdk/check_ptr.h"
#include "zdk/types.h"
#include "zdk/type_system_util.h"
#include "zdk/type_tags.h"
#include "zdk/zero.h"
#include "context.h"
#include "debug_out.h"
#include "errors.h"
#include "eval_sym.h"
#include "ident.h"
#include "interp.h"
#ifdef DEBUG
 #include "interp.h" // for debug_out
#endif
#include "type_spec.h"
#include "variant_impl.h"

using namespace std;


static void qualify_name(const string& prefix, string& name)
{
    size_t n = prefix.rfind("::");
    if ((n == string::npos) || (prefix.substr(n + 2) != name))
    {
        name = prefix + "::" + name;
    }
}


Ident::Ident(Interp* interp, const char* name, bool func, Register* reg)
    : PrimaryExpr(interp)
    , reg_(reg)
    , autoDeref_(true)
    , func_(func)
{
    assert(interp);

    assert(name);
    name_.assign(name);
    altName_.assign(name);
}


Ident::Ident(Interp* interp, const char* name, RefPtr<DebugSymbol> sym)
    : PrimaryExpr(interp)
    , sym_(sym)
    , autoDeref_(true)
    , func_(false)
{
    assert(interp);
    assert(name);

    altName_.assign(name);
    if (!sym)
    {
        name_ = altName_;
    }
    else
    {
        name_ = CHKPTR(sym->name())->c_str();
    }
}


Ident::Ident(RefPtr<Expr> prefix, RefPtr<Expr> ident)
    : PrimaryExpr(prefix->interp().get())
    , autoDeref_(true)
    , func_(false)
{
    //assert(prefix.get());
    assert(ident.get());
    assert(prefix->interp() == ident->interp());

    RefPtr<Ident> other;
    if (ident)
    {
        other = CHKPTR(interface_cast<Ident>(ident));
        if (other->reg_)
        {
            throw ParseError("cannot qualify a register");
        }
        // sym_ = other->sym_;
        name_ = other->name();
        func_ = other->func_;
    }

    altName_ = name_;
    // prefixed by namespace?
    if (Ident* ns = interface_cast<Ident*>(prefix.get()))
    {
        qualify_name(ns->alt_name(), name_);
    }
    else if (TypeName* owner = interface_cast<TypeName*>(prefix.get()))
    {
        // prefixed by a class name
        qualify_name(owner->name(), name_);
    }
    else
    {
        assert(false);
    }
    if (other && other->alt_name() == name_)
    {
        sym_ = other->sym_;
    }
    // altName_ = name_;
}


Ident::~Ident() throw()
{
}


void Ident::notify(Register* reg)
{
    assert(reg_);

    if (reg && strcmp(reg->name(), reg_->name()) == 0)
    {
        reg_ = reg;
    }
}


RefPtr<Variant> Ident::eval_reg(Context& ctxt)
{
    RefPtr<Variant> var;

    if (reg_)
    {
        if (Thread* thread = ctxt.thread())
        {
            thread->enum_user_regs(this);
            var = new VariantImpl;
            var->copy(reg_->value(), false);

            if (is_integer(*var))
            {
                set_type(GET_INT_TYPE(ctxt.type_system(), long));
            }
            else if (is_float(*var))
            {
                set_type(GET_FLOAT_TYPE(ctxt.type_system(), double));
            }
            else
            {
                assert(false);
            }
            return var;
        }
    }
    return var;
}


RefPtr<Variant> Ident::eval_impl(Context& ctxt)
{
    RefPtr<Variant> var = eval_reg(ctxt);
    if (var)
    {
        return var;
    }
    if (!sym_)
    {
        static const LookupOpt opts = (LKUP_FUNCS | LKUP_COLLECT);
        sym_ = ctxt.lookup_debug_symbol(name_.c_str(), opts);
    }

    var = eval_sym(ctxt, name_, sym_);

    assert(sym_); // eval_sym post-condition

    if (autoDeref_)
    {
        if (PointerType* ptr = interface_cast<PointerType*>(sym_->type()))
        {
            if (ptr->is_reference())
            {
                if (sym_->enum_children() == 0)
                {
                    sym_->read(&ctxt);
                }
                sym_ = sym_->nth_child(0);
                if (!sym_->value())
                {
                    sym_->read(&ctxt);
                }
                var = new VariantImpl(*sym_);
            }
        }
    }
    set_type(sym_->type());
    assert(!var || var->type_tag() != Variant::VT_NONE);

    return var;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
