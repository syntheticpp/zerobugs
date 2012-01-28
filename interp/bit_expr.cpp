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

#include <iostream>
#include "zdk/types.h"
#include "bit_expr.h"
#include "context.h"
#include "errors.h"
#include "variant_assign.h"
#include "variant_impl.h"


using namespace std;


BitExpr::BitExpr(Interp* interp, Oper oper, RefPtr<Expr> lhs, RefPtr<Expr> rhs)
    : BinaryExpr(interp, lhs, rhs), oper_(oper)
{
}


const char* BitExpr::operator_name() const
{
    switch (oper_)
    {
        case LEFT_SHIFT:  return "<<";
        case RIGHT_SHIFT: return ">>";
        case AND: return "AND";
        case OR:  return "OR";
        case XOR: return "XOR";
    }
    assert(false); // should never ever get here
    return "";
}


/**
 * Evaluate
 *  left shift
 *  right shift
 *  binary and
 *  binary or
 *  binar xor
 */
RefPtr<Variant> BitExpr::eval_impl(Context& context)
{
    RefPtr<Variant> lval, rval;

    eval_operands(context, lval, rval);

    if (oper_ == LEFT_SHIFT || oper_ == RIGHT_SHIFT)
    {
        if (lval->type_tag() == Variant::VT_OBJECT)
        {
            try_overloaded_operator(context);
        }
        else if (rval->type_tag() == Variant::VT_OBJECT)
        {
            try_standalone_operator(context);
        }
    }

    // Bitwise operations make sense for integer types only
    if (!is_integer(*lval) || !is_integer(*rval))
    {
        throw_invalid_types();
    }

    // Determine the bit size of the result
    IntType& intType = interface_cast<IntType&>(*lhs()->type());
    size_t n = intType.bit_size();
    if (intType.is_signed() && (n % 2) == 0)
    {
        assert(n);
        --n;
    }
    assert(n);
    --n;
    // compute mask for clearing unused bytes:
    uint64_t mask = 1ULL << n;
    mask = ((mask - 1) << 1) | 1;
    //clog << __func__ << ": mask=" << hex << mask << dec << endl;

    uint64_t rbits = rval->bits();
    // warn if shift rbits is negative
    if (is_signed_int(*rval) && (rbits & 0x8000000000000000ULL))
    {
        context.notify_warning_event("Warning: negative shift count");
    }

    RefPtr<DataType> unsignedType;
    if (!is_signed_int(*lval)) unsignedType = lhs()->type();
    if (!is_signed_int(*rval)) unsignedType = rhs()->type();

    if (lval->size() >= sizeof(int32_t))
    {
        // return type is the type of lhs
        set_type(lhs()->type());

        uint64_t lbits = lval->bits() & mask;
        //clog << __func__ << ": LBITS=" << hex << lbits << dec << endl;

        switch (oper_)
        {
        case LEFT_SHIFT:  lbits <<= rbits; break;
        case RIGHT_SHIFT: lbits >>= rbits; break;
        case AND: lbits &= rbits; if (unsignedType) set_type(unsignedType); break;
        case OR:  lbits |= rbits; if (unsignedType) set_type(unsignedType); break;
        case XOR: lbits ^= rbits; if (unsignedType) set_type(unsignedType); break;
        }

        //clog << __func__ << ": lbits=" << hex << lbits << dec << endl;
        lval = new VariantImpl;
        variant_assign(*lval, *type(), lbits);
    }
    else
    {
        // promote to int32_t or uint32_t
        RefPtr<DataType> tp;

        // if any of the types involved is unsigned and
        // the operation is not a bit shift, the result is
        // unsigned
        if (unsignedType && oper_ != LEFT_SHIFT && oper_ != RIGHT_SHIFT)
        {
            tp = context.get_int_type(false);
        }
        else
        {
            unsignedType.reset();
            tp = context.get_int_type(is_signed_int(*lval));
        }
        assert(tp);
        set_type(tp);

        uint32_t lbits = lval->bits() & mask;

        //clog << __func__ << ": lbits=" << hex << lbits << dec << endl;

        switch (oper_)
        {
        case LEFT_SHIFT:  lbits <<= rbits; break;
        case RIGHT_SHIFT: lbits >>= rbits; break;
        case AND: lbits &= rbits; break;
        case OR:  lbits |= rbits; break;
        case XOR: lbits ^= rbits; break;
        }

        //clog << __func__ << ": lbits=" << hex << lbits << dec << endl;
        lval = new VariantImpl;
        variant_assign(*lval, *type(), lbits);
    }

    return lval;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
