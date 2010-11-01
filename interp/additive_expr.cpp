//
// $Id: additive_expr.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdexcept>
#include <string>
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/types.h"
#include "zdk/type_system.h"
#include "context.h"
#include "debug_out.h"
#include "errors.h"
#include "interp.h"
#include "variant_assign.h"
#include "additive_expr.h"
#include "variant_impl.h"


using namespace std;


////////////////////////////////////////////////////////////////
AdditiveExpr::AdditiveExpr
(
    Interp*         interp,
    Operator        op,
    RefPtr<Expr>    lhs,
    RefPtr<Expr>    rhs
)
  : BinaryExpr(interp, lhs, rhs), operator_(op)
{
}


////////////////////////////////////////////////////////////////
void AdditiveExpr::add_float_int(
    const Variant&  lval,
    const Variant&  rval,
    Variant&        result,
    bool            negate)
{
    long double value = lval.long_double();

    switch (operator_)
    {
    case AdditiveExpr::PLUS:
        if (rval.type_tag() == Variant::VT_UINT64)
        {
            value += rval.uint64();
        }
        else
        {
            value += rval.int64();
        }
        break;

    case AdditiveExpr::MINUS:
        if (rval.type_tag() == Variant::VT_UINT64)
        {
            value -= rval.uint64();
        }
        else
        {
            value -= rval.int64();
        }
        if (negate)
        {
            value = -value;
        }
        break;
    }
    assert(type().get());
    variant_assign(result, *type(), value);
}


////////////////////////////////////////////////////////////////
void AdditiveExpr::add_uint64_int(
    const Variant&  lval,
    const Variant&  rval,
    Variant&        result,
    bool            negate)
{
    uint64_t value = lval.uint64();

    switch (operator_)
    {
    case AdditiveExpr::PLUS:
        if (rval.type_tag() == Variant::VT_UINT64)
        {
            value += rval.uint64();
        }
        else
        {
            value += rval.int64();
        }
        break;

    case AdditiveExpr::MINUS:
        if (rval.type_tag() == Variant::VT_UINT64)
        {
            value -= rval.uint64();
        }
        else
        {
            value -= rval.int64();
        }
        if (negate)
        {
            value = -value;
        }
        break;
    }
    assert(type().get());
    variant_assign(result, *type(), value);
}


////////////////////////////////////////////////////////////////
void AdditiveExpr::add_integers(
    const Variant&  lval,
    const Variant&  rval,
    Variant&        result)
{
    assert(is_integer(lval));
    assert(is_integer(rval));

    set_int_type(lval, rval);
    assert(type().get());

    int64_t value = lval.int64();
    if (operator_ == PLUS)
    {
        value += rval.int64();
    }
    else
    {
        value -= rval.int64();
    }
    assert(type().get());
    variant_assign(result, *type(), value);
}


////////////////////////////////////////////////////////////////
void AdditiveExpr::add_floats(
    const Variant&  lval,
    const Variant&  rval,
    Variant&        result)
{
    assert(is_float(lval));
    assert(is_float(rval));

    if (lhs()->type()->size() > rhs()->type()->size())
    {
        set_type(lhs()->type());
    }
    else
    {
        set_type(rhs()->type());
    }
    long double value = lval.long_double();
    if (operator_ == PLUS)
    {
        value += rval.long_double();
    }
    else
    {
        value -= rval.long_double();
    }
    variant_assign(result, *type(), value);
}


////////////////////////////////////////////////////////////////
void AdditiveExpr::add_pointer_int(
    const Variant&  lval,
    const Variant&  rval,
    VariantImpl&    result,
    DataType&       type)
{
    assert(lval.type_tag() == Variant::VT_POINTER);
    assert(is_integer(rval)
        || rval.type_tag() == Variant::VT_POINTER);

    PointerType& ptrType = interface_cast<PointerType&>(type);
    assert(ptrType.pointed_type());

    const size_t size = ptrType.pointed_type()->size();
    if (!size)
    {
        assert(interface_cast<VoidType*>(ptrType.pointed_type()));
        throw runtime_error("pointer of type ‘void *’ used in arithmetic");
    }
    const int factor = (operator_ == PLUS) ? 1 : -1;

    addr_t value = lval.pointer();
    DEBUG_OUT << "value=" << (void*)value << endl;
    if (rval.type_tag() == Variant::VT_POINTER)
    {
        assert(operator_ == MINUS);
        assert(size);
        value = (value - rval.pointer()) / size;

        if (RefPtr<Interp> in = interp())
        {
            TypeSystem& typeSys = in->context().type_system();
            RefPtr<DataType> diffType =
                typeSys.get_int_type(shared_string("size_t").get(),
                                     typeSys.word_size(),
                                     false);
            set_type(diffType);
            variant_assign(result, *diffType, value);
        }
    }
    else if (rval.type_tag() == Variant::VT_UINT64)
    {
        value += factor * rval.uint64() * size;
        set_type(&type);
        variant_assign(result, type, value);
    }
    else
    {
        value += factor * rval.int64() * size;
        set_type(&type);
        variant_assign(result, type, value);
    }
    result.set_encoding(lval.encoding());
}


////////////////////////////////////////////////////////////////
RefPtr<Variant> AdditiveExpr::eval_impl(Context& ctxt)
{
    RefPtr<Variant> lval, rval;
    eval_operands(ctxt, lval, rval);
    RefPtr<VariantImpl> result = new VariantImpl;
    //
    // and now for an ugly display of brute-force...
    //
    if (lval->type_tag() == Variant::VT_POINTER)
    {
        if (rval->type_tag() == Variant::VT_POINTER)
        {
            // cannot add 2 pointers
            if (operator_ != MINUS)
            {
                throw_invalid_types();
            }
            add_pointer_int(*lval, *rval, *result, *lhs()->type());
        }
        else if (is_integer(*rval))
        {
            add_pointer_int(*lval, *rval, *result, *lhs()->type());
        }
        else if (is_float(*rval))
        {
            // can't add/subtract float from pointer
            throw_invalid_types();
        }
    }
    else if (lval->type_tag() == Variant::VT_OBJECT)
    {
        try_overloaded_operator(ctxt);
    }
    else if (rval->type_tag() == Variant::VT_OBJECT)
    {
        try_standalone_operator(ctxt);
    }
    else if (is_integer(*lval))
    {
        if (rval->type_tag() == Variant::VT_POINTER)
        {
            if (operator_ == MINUS)
            {
                // cannot subtract pointer from integer
                throw_invalid_types();
            }
            else
            {
                add_pointer_int(*rval, *lval, *result, *rhs()->type());
            }
        }
        else if (is_integer(*rval))
        {
            if (lval->type_tag() == Variant::VT_UINT64)
            {
                // the resulting expression is of type uint64
                set_type(lhs()->type());
                add_uint64_int(*lval, *rval, *result);
            }
            else if (rval->type_tag() == Variant::VT_UINT64)
            {
                // the resulting expression is of type uint64
                set_type(rhs()->type());
                add_uint64_int(*rval, *lval, *result, true);
            }
            else
            {
                // none of the operands is uint64_t
                add_integers(*lval, *rval, *result);
            }
        }
        else if (is_float(*rval))
        {
            set_type(rhs()->type());
            add_float_int(*rval, *lval, *result, true);
        }
    }
    else if (is_float(*lval))
    {
        if (rval->type_tag() == Variant::VT_POINTER)
        {
            // can't do pointer arithmetic with floats
            throw_invalid_types();
        }
        else if (is_integer(*rval))
        {
            set_type(lhs()->type());
            add_float_int(*lval, *rval, *result);
        }
        else if (is_float(*rval))
        {
            add_floats(*lval, *rval, *result);
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
const char* AdditiveExpr::operator_name() const
{
    switch (operator_)
    {
    case MINUS: return "-";
    case PLUS:  return "+";
    }
    assert(false);
    return "";
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
