//
// $Id: constant.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <iomanip>
#include <sstream>
#include "context.h"
#include "debug_out.h"
#include "errors.h"
#include "interp.h"
#include "zdk/types.h"
#include "zdk/type_system_util.h"
#include "constant.h"
#include "debug_out.h"
#include "variant_impl.h"

using namespace std;
using namespace Platform;


Constant::Constant(Interp* interp)
    : PrimaryExpr(interp)
    , var_(new VariantImpl)
{
}


Constant::Constant(Interp* interp, DebugSymbol& dsym)
    : PrimaryExpr(interp)
    , var_(new VariantImpl(dsym))
{
    set_type(dsym.type());
}


Constant::~Constant() throw()
{
}


void Constant::set_variant(Variant& var)
{
    var_ = &var;

    if (var.debug_symbol())
    {
        assert(var.debug_symbol()->type());
        set_type(var.debug_symbol()->type());
    }
}


RefPtr<Expr> Constant::clone(Interp* interp) const
{
    assert(var_);
    assert(var_->type_tag() != Variant::VT_NONE);

    RefPtr<Constant> c = new Constant(interp, var_);
    c->set_type(this->type());

    return c;
}


IntegerConstant::~IntegerConstant() throw()
{
}


IntegerConstant::IntegerConstant(Interp* interp, char val)
    : Constant(interp)
{
    set_value(val, Variant::VT_INT8);
    assert(interp);
    TypeSystem& types = interp->context().type_system();
    set_type(get_int_type(types, (char*)0, "char"));
}


IntegerConstant::IntegerConstant
(
    Interp* interp,
    const char* yytext,
    int base
) : Constant(interp)
{
    assert(interp);

    TypeSystem& types = interp->context().type_system();
    istringstream is(yytext);

    switch (base)
    {
    default: assert(false); break;
    case 10: break;
    case 16: is >> hex; break;
    case  8: is >> oct; break;
    }

    unsigned long long val = 0;
    is >> val;

    const bool is32Bit = interp->is_32_bit();

    // strlen("0x") == 2, plus 2 HEX characters for each digit
    //const size_t maxStrLen = is32Bit ? (2 + 4 * 2) : (2 + sizeof(val) * 2);
    const size_t maxStrLen = (2 + sizeof(val) * 2);

    string suffix;

    bool isUnsigned = false;
    int  lcount = 0;

    if (is >> suffix)
    {
        for (string::const_iterator i(suffix.begin()); i != suffix.end(); ++i)
        {
            const char c = *i;

            if (c == 'L' || c == 'l')
            {
                if (lcount++ == 2)
                {
                    throw ParseError("too many Ls in integer constant");
                }
            }
            else if (c == 'U' || c == 'u')
            {
                isUnsigned = true;
            }
        }
    }
    else if (base == 16)
    {
        assert(yytext[0] == '0');
        assert(yytext[1] == 'x' || yytext[1] == 'X');

        if ((yytext[2] == '0') && strlen(yytext) <= maxStrLen)
        {
            isUnsigned = true;
        }
    }
    if (val > numeric_limits<uint32_t>::max())
    {
        if (is32Bit && lcount < 2)
        {
            // mimic the compiler's behavior on overflow
            throw runtime_error("integer constant is too large for 'long' type");
        }
        lcount = 2; // same as LL (long long)
    }
    RefPtr<DataType> type;
    Variant::TypeTag tag = Variant::VT_INT;

#if (__WORDSIZE == 64)
    if (lcount && !is32Bit)
    {
        lcount = 2; // both "long" and "long long" are 64-bit
    }
#endif
    switch (lcount)
    {
    case 2:
        if (isUnsigned)
        {
            tag = Variant::VT_ULONGLONG;
            type = GET_INT_TYPE(types, unsigned long long);
        }
        else
        {
            tag = Variant::VT_LONGLONG;
            type = GET_INT_TYPE(types, long long);
        }
        set_value(val, tag);
        break;

    case 1:

#if (__WORDSIZE == 64)
        if (is32Bit)
        {
            if (isUnsigned)
            {
                tag = Variant::VT_UINT;
                type = GET_INT_TYPE(types, unsigned int);
                set_value(static_cast<unsigned int>(val), tag);
            }
            else
            {
                tag = Variant::VT_INT;
                type = GET_INT_TYPE(types, int);
                set_value(static_cast<int>(val), tag);
            }

            break;
        }
#endif
        if (isUnsigned)
        {
            tag = Variant::VT_ULONG;
            type = GET_INT_TYPE(types, unsigned long);
            set_value(static_cast<unsigned long>(val), tag);
        }
        else
        {
            tag = Variant::VT_LONG;
            type = GET_INT_TYPE(types, long);
            set_value(static_cast<long>(val), tag);
        }
        break;

    case 0:
        if (isUnsigned)
        {
            tag = Variant::VT_UINT;
            type = GET_INT_TYPE(types, unsigned int);
            set_value(static_cast<unsigned int>(val), tag);
        }
        else
        {
            tag = Variant::VT_INT;
            type = GET_INT_TYPE(types, int);
            set_value(static_cast<int>(val), tag);
        }
        break;

    default:
        assert(false);
    }
    DEBUG_OUT << type->name() << " (tag=" << tag << " long=" << lcount << ")\n";

    assert(type.get());
    set_type(type);
}


FloatingPointConstant::~FloatingPointConstant() throw()
{
}



FloatingPointConstant::FloatingPointConstant(Interp* interp, const char* yytext)
    : Constant(interp)
{
    assert(interp);
    TypeSystem& types = interp->context().type_system();
    istringstream is(yytext);

    long double value = 0;

    is >> value;

    string suffix;

    Variant::TypeTag tag = Variant::VT_DOUBLE;

    if (is >> suffix)
    {
        for (string::const_iterator i(suffix.begin()); i != suffix.end(); ++i)
        {
            if (*i == 'f' || *i == 'F')
            {
                if (tag != Variant::VT_DOUBLE)
                {
                    throw ParseError("both F and L specified in constant");
                }
                tag = Variant::VT_FLOAT;
            }
            else if (*i == 'l' || *i == 'L')
            {
                //if (tag == Variant::VT_FLOAT)
                if (tag != Variant::VT_DOUBLE)
                {
                    throw ParseError("both F and L specified in constant");
                }
                tag = Variant::VT_LONG_DOUBLE;
            }
        }
    }

    RefPtr<DataType> type;

    if (tag == Variant::VT_FLOAT)
    {
        set_value(static_cast<float>(value), tag);
        type = GET_FLOAT_TYPE(types, float);
    }
    else if (tag == Variant::VT_DOUBLE)
    {
        set_value(static_cast<double>(value), tag);
        type = GET_FLOAT_TYPE(types, double);
    }
    else
    {
        set_value(value, tag);
        type = GET_FLOAT_TYPE(types, long double);
    }
    set_type(type);
}



StringLiteralConstant::StringLiteralConstant(Interp* interp, const char* yytext)
    : Constant(interp)
{
    assert(interp);
    assert(yytext);
    assert(*yytext == '\"');

    assert(strlen(yytext) >= 2);
    string text(yytext + 1, yytext + strlen(yytext) - 1);

    TypeSystem& types = interp->context().type_system();

    RefPtr<DataType> type = types.get_string_type();

    RefPtr<DebugSymbol> dsym = interp->context().new_const(*type, text);
    assert(dsym->is_constant());
    assert(dsym->value());

    RefPtr<Variant> var = new VariantImpl(*dsym);

    set_variant(*var);
}


StringLiteralConstant::~StringLiteralConstant() throw()
{
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
