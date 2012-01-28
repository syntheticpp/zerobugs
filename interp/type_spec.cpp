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
#include <string>
#include "zdk/shared_string_impl.h"
#include "zdk/types.h"
#include "zdk/type_system.h"
#include "context.h"
#include "interp.h"
#include "errors.h"
#include "type_spec.h"
#include "variant_impl.h"


using namespace std;


TypeSpec::TypeSpec(Interp* interp, Flags flags, const char* name)
    : Expr(interp), flags_(flags)
{
    if (name)
    {
        name_.assign(name);
    }
    else if (interp)
    {
        name_ = interp->yytext();
    }
}


TypeSpec::~TypeSpec() throw()
{
}


RefPtr<Variant> TypeSpec::eval_impl(Context& context)
{
    RefPtr<Variant> empty = new VariantImpl;

    if (!type())
    {
        RefPtr<DataType> type = context.lookup_type(name().c_str());

        assert(type);
#ifdef DEBUG
        if (type)
        {
            clog << this->name() << ": " << type->name() << endl;
        }
#endif
        set_type(type);
    }
    return empty;
}


TypeName::TypeName(Interp* interp, RefPtr<DataType> type)
    : TypeSpec(interp, TF_TYPENAME, (interp ? 0 : type->name()->c_str()))
{
    assert(type);

    set_type(type);
}


TypeSpecList::TypeSpecList(Interp* interp) : Expr(interp), flags_(0)
{
}


TypeSpecList::~TypeSpecList() throw()
{
}


void TypeSpecList::push_back(RefPtr<Expr> expr)
{
    assert(expr); // precondition

    TypeSpec& spec = interface_cast<TypeSpec&>(*expr);

    TypeSpec::Flags flags = spec.flags();
    validate_and_set(flags);

    RefPtr<DataType> type = spec.type();
    if (type.get())
    {
        assert(flags & TypeSpec::TF_TYPENAME);
        set_type(type);
    }
    else
    {
        //assert(flags & TypeSpec::TF_TYPENAME == 0);
    }

    if (!name_.empty())
    {
        name_ += " ";
    }
    name_ += spec.name();
}


void TypeSpecList::validate_and_set(TypeSpec::Flags flags)
{
    switch (flags)
    {
    case TypeSpec::TF_SHORT:
        if ((flags_ & TypeSpec::TF_CHAR) == TypeSpec::TF_CHAR)
        {
            throw ParseError("short specifier invalid for char");
        }
        if ((flags_ & TypeSpec::TF_SHORT) == TypeSpec::TF_SHORT)
        {
            throw ParseError("too many 'short' specifiers");
        }
        break;

    case TypeSpec::TF_UNSIGNED:
    case TypeSpec::TF_SIGNED:
        break;

    case TypeSpec::TF_LONG:
        if (flags_ & TypeSpec::TF_VOID)
        {
            throw ParseError("long specifier invalid for void");
        }
        if ((flags_ & TypeSpec::TF_CHAR) == TypeSpec::TF_CHAR)
        {
            throw ParseError("long specifier invalid for char");
        }
        if (flags_ & TypeSpec::TF_LONGLONG)
        {
            // long long long is illegal
            throw ParseError("too many 'long' specifiers");
        }
        else if (flags_ & TypeSpec::TF_LONG) // long long?
        {
            flags_ |= TypeSpec::TF_LONGLONG;
        }
        break;

    case TypeSpec::TF_LONGLONG:
        break;

    case TypeSpec::TF_INT:
        if (flags_ & (TypeSpec::TM_NON_INT))
        {
            throw ParseError("more than one type specified");
        }
        if ((flags_ & TypeSpec::TF_CHAR) == TypeSpec::TF_CHAR)
        {
            throw ParseError("more than one type specified");
        }
        if ((flags_ & TypeSpec::TF_INT) == TypeSpec::TF_INT)
        {
            throw ParseError("more than one type specified");
        }
        break;

    case TypeSpec::TF_CHAR:
        if ((flags_ & TypeSpec::TF_SHORT) == TypeSpec::TF_SHORT)
        {
            throw ParseError("short specifier invalid for char");
        }
        if (flags_ & TypeSpec::TF_LONG)
        {
            throw ParseError("long specifier invalid for char");
        }
        if (flags_ & (TypeSpec::TF_VOID | TypeSpec::TM_SIZE))
        {
            throw ParseError("more than one type specified");
        }
        break;

    case TypeSpec::TF_FLOAT:
        // fallthru

    case TypeSpec::TF_DOUBLE:
        if ((flags_ & TypeSpec::TF_SHORT) == TypeSpec::TF_SHORT)
        {
            throw ParseError("short, signed or unsigned invalid for non-int");
        }
        if (flags_ & (TypeSpec::TM_SIZE | TypeSpec::TF_VOID))
        {
            throw ParseError("more than one type specified");
        }
        break;

    case TypeSpec::TF_VOID:
        // fallthru

    case TypeSpec::TF_TYPENAME:
        if (flags_ & ~(TypeSpec::TF_CONST | TypeSpec::TF_VOLATILE))
        {
            throw ParseError("more than one type specified");
        }
        break;

    case TypeSpec::TF_CONST:
    case TypeSpec::TF_VOLATILE:
        break;

    default:
        assert(false);
    }
    flags_ |= flags;
}


/**
 * Perform more validation, then determine the type
 */
RefPtr<Variant> TypeSpecList::eval_impl(Context& ctxt)
{
    TypeSystem& typeSys = ctxt.type_system();
    RefPtr<Variant> dummy = new VariantImpl;

    // signed and unsigned can only be applied to ints
    const int f = (flags_ & (TypeSpec::TF_SIGNED | TypeSpec::TF_UNSIGNED));
    if (f)
    {
        if (f == (TypeSpec::TF_SIGNED | TypeSpec::TF_UNSIGNED))
        {
            throw EvalError("signed and unsigned both specified");
        }
        if (flags_ & TypeSpec::TM_NON_INT)
        {
            throw EvalError("short, signed or unsigned invalid for non-int");
        }
        flags_ |= TypeSpec::TM_INT;
    }
    // short only applies to ints
    if ((flags_ & TypeSpec::TF_SHORT) && (flags_ & TypeSpec::TM_NON_INT))
    {
         throw EvalError("short, signed or unsigned invalid for non-int");
    }
    // long only applies to int, long, and double
    if (flags_ & TypeSpec::TF_LONG)
    {
        if ((flags_ & TypeSpec::TF_SHORT) == TypeSpec::TF_SHORT)
        {
            throw EvalError("long and short cannot be specified together");
        }
        if ((flags_ & TypeSpec::TF_FLOAT) == TypeSpec::TF_FLOAT)
        {
            throw EvalError("long invalid for float (did you mean double?)");
        }
        if ((flags_ & TypeSpec::TF_DOUBLE) == TypeSpec::TF_DOUBLE)
        {
            if (flags_ & TypeSpec::TF_LONGLONG)
            {
                throw ParseError("too many 'long' specifiers");
            }
            flags_ &= ~TypeSpec::TM_SIZE;
            flags_ |= sizeof (long double);
        }
        else if (flags_ & TypeSpec::TF_LONGLONG)
        {
            flags_ |= TypeSpec::TM_INT;
            flags_ &= ~TypeSpec::TM_SIZE;
            flags_ |= sizeof (long long);
        }
        else if ((flags_ & TypeSpec::TM_INT) == 0)
        {
            flags_ |= TypeSpec::TM_INT;
        }
    }
    if (flags_ & TypeSpec::TF_TYPENAME)
    {
        if (!type())
        {
            RefPtr<DataType> type = ctxt.lookup_type(name().c_str());
            if (!type)
                clog << "type not found: " <<name() << endl;
            set_type(type);
        }
    }
    else if (flags_ & TypeSpec::TM_INT)
    {
        assert((flags_ & TypeSpec::TM_TYPE) == TypeSpec::TM_INT);

        bool isSigned = true;
        if (flags_ & TypeSpec::TF_UNSIGNED)
        {
            assert((flags_ & TypeSpec::TF_SIGNED) == 0);
            isSigned = false;
        }
        size_t size = (flags_ & TypeSpec::TM_SIZE);

        switch (size)
        {
        case 0:
            size = sizeof(Platform::word_t);// default to machine-word
            break;
        case sizeof(char):
            assert((flags_ & TypeSpec::TF_CHAR) == TypeSpec::TF_CHAR);
            break;
        case sizeof(short):
            assert((flags_ & TypeSpec::TF_SHORT) == TypeSpec::TF_SHORT);
            break;
        case sizeof(int32_t):
            break;
        case sizeof(int64_t):
            assert(flags_ & TypeSpec::TF_LONGLONG);
            break;
        default: assert(false);
        }

        RefPtr<DataType> type =
            typeSys.get_int_type(
                shared_string(name_).get(),
                Platform::byte_size * size, // size in bits
                isSigned);
        assert(type.get());
        set_type(type);
    }
    else if (flags_ & TypeSpec::TM_FLOAT)
    {
        assert((flags_ & TypeSpec::TM_TYPE) == TypeSpec::TM_FLOAT);
        //string tname;
        const size_t size = (flags_ & TypeSpec::TM_SIZE);
        switch (size)
        {
        default: assert(false); break;
        case sizeof(float):
        case sizeof(double):
        case sizeof(long double):
            break;
        }
        RefPtr<DataType> type =
            typeSys.get_float_type(
                shared_string(name_).get(), size);
        set_type(type);
    }
    else if (flags_ & TypeSpec::TF_VOID)
    {
        assert(flags_ == TypeSpec::TF_VOID);
        set_type(typeSys.get_void_type());
    }

    if (type())
    {
        RefPtr<DataType> type = this->type();

        if (flags_ & TypeSpec::TF_VOLATILE)
        {
            type = typeSys.get_qualified_type(type.get(), QUALIFIER_VOLATILE);
        }
        if (flags_ & TypeSpec::TF_CONST)
        {
            type = typeSys.get_qualified_type(type.get(), QUALIFIER_CONST);
        }
        set_type(type);
    }
    return dummy;
}


RefPtr<Expr> verify_class_or_struct(const RefPtr<Expr>& expr)
{
    assert(expr);
    assert(interface_cast<TypeSpec*>(expr.get()));
    assert(expr->type().get());
    assert(expr->type()->name());

    string typeName(expr->type()->name()->c_str());

    ClassType* klass = interface_cast<ClassType*>(expr->type().get());
    if (!klass)
    {
        throw ParseError(typeName + " is not a class nor a struct");
    }
    if (klass->is_union())
    {
        throw ParseError(typeName + " is a union");
    }
    return expr;
}


RefPtr<Expr> verify_union(const RefPtr<Expr>& expr)
{
    assert(expr);
    assert(interface_cast<TypeSpec*>(expr.get()));
    assert(expr->type().get());
    assert(expr->type()->name());

    string typeName(expr->type()->name()->c_str());

    ClassType* klass = interface_cast<ClassType*>(expr->type().get());
    if (!klass || !klass->is_union())
    {
        throw ParseError(typeName + " is not a union");
    }
    return expr;
}


RefPtr<Expr> verify_enum(const RefPtr<Expr>& expr)
{
    assert(expr);
    assert(interface_cast<TypeSpec*>(expr.get()));
    assert(expr->type().get());
    assert(expr->type()->name());

    string typeName(expr->type()->name()->c_str());

    if (!interface_cast<EnumType*>(expr->type().get()))
    {
        throw ParseError(typeName + " is not an enumerated type");
    }
    return expr;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
