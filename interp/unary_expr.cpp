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
#include <iostream>
#include <limits>
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/types.h"
#include "zdk/type_system_util.h"
#include "zdk/variant_util.h"
#include "zdk/zero.h" // thread->is_32_bit
#include "typez/public/type_tags.h" // put<>
#include "call_setup.h"
#include "debug_out.h"
#include "eval_inc_dec.h"
#include "errors.h"
#include "interp.h"
#include "lookup_methods.h"
#include "unary_expr.h"
#include "variant_assign.h"
#include "variant_impl.h"

using namespace std;


UnaryExpr::UnaryExpr
(
    Interp*         interp,
    Operator        oper,
    RefPtr<Expr>    arg,
    Source          src
)
  : Expr(interp), oper_(oper), arg_(arg), src_(src)
{
    assert(arg_.get());
    assert(oper != REFERENCE || src_ == INTERNAL);
}


UnaryExpr::~UnaryExpr() throw()
{
}


RefPtr<Variant> UnaryExpr::eval_impl(Context& context)
{
    // precondition
    if (!arg_)
    {
        throw logic_error("unary operator has null argument");
    }

    RefPtr<Variant> var = arg_->eval(context);
    // arg_->eval post-conditions:
    if (!arg_->type())
    {
        throw logic_error("unary argument has null type");
    }
    if (!var && oper_ != SIZE_OF)
    {
        throw logic_error("error evaluating unary argument");
    }
    if (src_ != INTERNAL && try_overloaded_operator(context, var))
    {
        // the result doesn't really matter here; we're going
        // to call the overloaded operator, and the expression's
        // result will be filled out asynchronously, when the
        // operator call returns -- see on_call_return.cpp
        return 0;
    }

    switch (oper_)
    {
    case REFERENCE:
    case ADDR: var = eval_addr(context, *var);
        break;

    case DEREF: var = eval_deref(context, *var);
        break;

    case PLUS:  // anything to do here?
        break;

    case MINUS: var = eval_minus(context, *var);
        break;

    case BITNEG: var = eval_bit_complement(context, *var);
        break;

    case NEGATE: var = eval_negate(context, *var);
        break;

    case INCREMENT:
        var = eval_inc_dec(context, *arg_->type(), *var, true);
        // make the result expression of the same type as the arg
        set_type(arg_->type());
        break;

    case DECREMENT:
        var = eval_inc_dec(context, *arg_->type(), *var, false);
        set_type(arg_->type());
        break;

    case SIZE_OF: var = eval_sizeof(context);
        break;
    }
    return var;
}


RefPtr<Variant>
UnaryExpr::eval_addr(Context& context, const Variant& var)
{
    RefPtr<Variant> result;
    if (DebugSymbol* symbol = var.debug_symbol())
    {
        if (!symbol->type())
        {
            throw logic_error("unknown symbol type");
        }

        RefPtr<DataType> type;
        if (oper_ == ADDR)
        {
            type = context.type_system().get_pointer_type(symbol->type());
        }
        else
        {
            assert(oper_ == REFERENCE);
            assert(src_ == INTERNAL);
            type = context.type_system().get_reference_type(symbol->type());
        }
        assert(type.get());
        set_type(type);

        ostringstream val;

        val << symbol->addr();
        RefPtr<DebugSymbol> sym = context.new_const(*type, val.str());

        if (!sym)
        {
            result = new VariantImpl;
            if (context.thread()->is_32_bit())
            {
                uint32_t addr(symbol->addr());
                put(result.get(), addr, Variant::VT_POINTER);
            }
            else
            {
                put(result.get(), symbol->addr(), Variant::VT_POINTER);
            }
        }
        else
        {
            result = new VariantImpl(*sym);
        }
    }
    else
    {
        throw EvalError("non-lvalue in unary &");
    }
    return result;
}



RefPtr<Variant>
UnaryExpr::eval_deref(Context& context, const Variant& var)
{
    RefPtr<DebugSymbol> symbol = var.debug_symbol();

    if (var.type_tag() != Variant::VT_POINTER)
    {
        if (!arg_->is_type_strict() && is_integer(var))
        {
            // hack, mainly to make evaluation of errno work;
            // GCC #defines errno as (*__errno_location())
            RefPtr<DataType> intType = GET_INT_TYPE(context.type_system(), long);

            RefPtr<PointerType> ptrType =
                context.type_system().get_pointer_type(intType.get());
            symbol = context.new_temp_ptr(*ptrType, var.bits());
            DEBUG_OUT << symbol->addr() << endl;
        }
        else
        {
            throw EvalError("invalid type argument of unary *");
        }
    }

    if (!symbol)
    {
        assert(arg_->type().get()); // caller should've already checked

        PointerType& ptrType = interface_cast<PointerType&>(*arg_->type());
        symbol = context.new_temp_ptr(ptrType, var.pointer());
    }
    if (!symbol)
    {
        // should only happen in the unit test, since the TestContext
        // does not create tmp pointers
        throw logic_error("null lvalue in unary *");
    }
    if (!symbol->type())
    {
        throw logic_error("null symbol type unary *");
    }
    PointerType* ptrType = interface_cast<PointerType*>(symbol->type());
    if (!ptrType)
    {
        cerr << __func__ << ": type=" << symbol->type()->name() << endl;
        throw EvalError("non-pointer symbol type in unary *");
    }
    if (ptrType->is_reference() && src_ != INTERNAL)
    {
        throw EvalError("non-pointer symbol type in unary *");
    }

    symbol->read(&context);

    // this may happen if the pointer is pointing to an
    // address that cannot be read
    if (symbol->enum_children() == 0)
    {
        throw EvalError("could not dereference expression");
    }
    DEBUG_OUT << symbol->value() << endl;

    // todo: the variable name is same as the type, that ok?
    symbol = symbol->nth_child(0);
    if (symbol)
    {
        symbol->read(&context);
        set_type(CHKPTR(symbol->type()));
        return new VariantImpl(*symbol);
    }
    return RefPtr<Variant>();
}


/**
 * Evaluate unary minus
 */
RefPtr<Variant>
UnaryExpr::eval_minus(Context& context, const Variant& v)
{
    RefPtr<Variant> result = new VariantImpl;
    set_type(arg_->type());

    if (is_float(v))
    {
        const long double x = -v.long_double();
        variant_assign(*result, *type(), x);
    }
    else if (is_integer(v))
    {
        if (v.type_tag() == Variant::VT_UINT64)
        {
            const uint64_t x = -v.uint64();
            variant_assign(*result, *type(), x);
        }
        else
        {
            const int64_t x = -v.int64();
            variant_assign(*result, *type(), x);
        }
        // todo: handle bit-fields here
    }
    else
    {
        throw EvalError("invalid type argument of unary -");
    }
    return result;
}


RefPtr<Variant>
UnaryExpr::eval_bit_complement(Context& context, const Variant& v)
{
    if (!is_integer(v))
    {
        throw EvalError("invalid type argument to bit-complement");
    }
    set_type(arg_->type());

    //uint64_t mask = 1ULL << (type()->bit_size() - 1);
    uint64_t mask = 1ULL << (type()->size() * Platform::byte_size - 1);
    mask = ((mask - 1) << 1) | 1;
    //clog << __func__ << ": mask=" << hex << mask << dec << endl;

    const uint64_t bits = ~v.bits() & mask;

    RefPtr<VariantImpl> result = new VariantImpl;
    variant_assign(*result, *type(), bits);
    return result;
}


RefPtr<Variant> UnaryExpr::eval_negate(Context& context, const Variant& v)
{
/*
    switch (v.type_tag())
    {
    case Variant::VT_NONE:
    case Variant::VT_OBJECT:
    case Variant::VT_ARRAY:
        throw EvalError("invalid type argument to logical negation");

    default:
        break;
    }
 */
    RefPtr<Variant> result = new VariantImpl;
    RefPtr<DataType> intType = context.get_int_type();

    set_type(intType);
    int value = 0;

    if (is_integer(v))
    {
        value = (v.bits() == 0);
    }
    else if (is_float(v))
    {
        value = (v.long_double() < numeric_limits<long double>::epsilon());
    }
    else if (v.type_tag() == Variant::VT_POINTER)
    {
        value = (v.pointer() == 0);
    }
    else
    {
        throw EvalError("invalid type argument to logical negation");
    }
    variant_assign(*result, *intType, value);
    return result;
}


RefPtr<Variant> UnaryExpr::eval_sizeof(Context& context)
{
    ostringstream os;
    os << arg_->type()->size();

    RefPtr<DataType> sizeType = context.get_int_type(false);
    RefPtr<DebugSymbol> sym = context.new_const(*sizeType, os.str());

    return new VariantImpl(*sym);
}


bool
UnaryExpr::try_overloaded_operator(Context& ctxt, RefPtr<Variant> var)
{
    if (!var)
    {
        return false;
    }
    if (var->type_tag() == Variant::VT_OBJECT)
    {
        ClassType& klass = interface_cast<ClassType&>(*arg_->type());
        string op = "operator";
        switch (oper_)
        {
        case REFERENCE: assert(src_ == INTERNAL);
        case SIZE_OF: return false;
        case INCREMENT: op += "++"; break;
        case DECREMENT: op += "--"; break;
        default: op += oper_; break;
        }

        RefPtr<DebugSymbol> fun = lookup_methods(ctxt, klass, op);
        if (!fun)
        {
            return false;
        }
        ExprList args;
        args.push_back(new UnaryExpr(
            interp().get(), UnaryExpr::ADDR, arg_, UnaryExpr::INTERNAL));
        RefPtr<CallSetup> setup =
            get_call_setup(ctxt, *this, fun.get(), &args);
        if (setup.get())
        {
            assert(setup->reader() == fun->reader());
            ctxt.call_function(*setup);
        }
        return true;
    }
    return false;
}


RefPtr<Expr> UnaryExpr::clone(Interp* interp) const
{
    return new UnaryExpr(interp, oper_, arg_->clone(interp), src_);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
