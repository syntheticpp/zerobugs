//
// $Id: cast_expr.cpp 729 2010-10-31 07:00:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Evaluation of cast expressions. C-style casts, C++ casts
// and implicit casts are supported.
//
#include <cassert>
#include <stdexcept>
#include "dharma/symbol_util.h"
#include "debug_out.h"
#include "errors.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/types.h"
#include "zdk/type_system.h"
#include "zdk/variant_util.h"
#include "typez/public/adjust_base.h"
#include "typez/public/remove_qual.h"
#include "variant_convert.h"
#include "call_setup.h"
#include "cast_expr.h"
#include "constant.h"
#include "context.h"
#include "debug_out.h"
#include "ident.h"
#include "interp.h"
#include "type_spec.h"
#include "unary_expr.h"
#include "variant_impl.h"


using namespace std;


const char* conv_type[] =
{
        "C-style",                  // CAST_NONE,
        "identity",                 // CAST_IDENTITY,
        "cv-qualified",             // CAST_CV_QUALIFIED,
        "const_cast",               // CAST_CONST_CAST,
        "derived-to-base",          // CAST_DERIVED_TO_BASE,
        "base-to-derived",          // CAST_BASE_TO_DERIVED,
        "ctor or operator",         // CAST_USER_CONVERSION,
        "implicit",                 // CAST_IMPLICIT
};

string cast_type(CastExpr::Conversion conv)
{
    // assert(sizeof(conv_type)/sizeof(conv_type[0]) > (size_t)conv);
    return conv_type[conv];
}


#if 0 //  DEBUG
#define LOG_CAST_TYPE() DEBUG_OUT << cast_type(conv_) << endl

#define TRACE_CAST_(to, from) \
    if (Interp::debug_enabled()) { \
    clog << "----- " << __func__ << " -----\n";     \
    clog << __FILE__ << ":" << __LINE__ << endl;    \
    clog << "type()=" << type()->name() << endl;    \
    clog << "TARGET=" << (to)->_name();             \
    clog << ": " << (to)->name() << endl;           \
    clog << "SOURCE=" << (from)->_name();           \
    clog << ": " << (from)->name() << endl << endl; }

#define TRACE_CAST() TRACE_CAST_(typeTo, typeFrom)
#else
 #define LOG_CAST_TYPE()
 #define TRACE_CAST()
 #define TRACE_CAST_(x,y)
#endif


////////////////////////////////////////////////////////////////
CastExpr::CastExpr
(
    RefPtr<Expr> castTo,
    RefPtr<Expr> castFrom,
    bool constCastOK
)
  : Expr(castFrom->interp().get())
  , castTo_(castTo)
  , castFrom_(castFrom)
  , conv_(CAST_NONE)
  , constCastOK_(constCastOK)
  , isDynamic_(false)
{
    assert(castFrom_.get());
    assert(this->interp());

    if (Ident* ident = interface_cast<Ident*>(cast_from().get()))
    {
        ident->disable_auto_deref();
    }
}


////////////////////////////////////////////////////////////////
RefPtr<Variant> CastExpr::eval_sub_expressions(Context& context)
{
    // eval both cast-to and cast-from; the result of cast-to
    // is ignored, since we only care for the type of the expr
    RefPtr<Variant> var = castTo_->eval(context);

    var = castFrom_->eval(context);
    if (!var)
    {
        throw logic_error("cast-from argument yielded null");
    }
    // types should be known at this point;
    // if not, then it's a programming mistake
    if (var->type_tag() == Variant::VT_NONE || !castFrom_->type())
    {
        throw logic_error("unknown cast-from type");
    }
    if (!castTo_->type())
    {
        throw logic_error("unknown cast-to type");
    }
    // optimistically assume it's a legal cast --
    // the type of the result is the type we're casting to
    set_type(castTo_->type());

#if DEBUG
    if (Interp::debug_enabled())
    {
        clog << _name() << " " << castFrom_->type()->name();
        clog << " --> " << castTo_->type()->name() << endl;
    }
#endif // DEBUG
    return var;
}


////////////////////////////////////////////////////////////////
RefPtr<Variant> CastExpr::eval_impl(Context& context)
{
    RefPtr<Variant> var = eval_sub_expressions(context);
    assert(var.get()); // post-condition
    assert(type() == castTo_->type()); // post-condition

    RefPtr<DataType> typeFrom = castFrom_->type();

    conv_ = try_id_and_qualified(type(), typeFrom, constCastOK_);
    if (conv_ != CAST_NONE)
    {
        if (conv_ != CAST_IDENTITY)
        {
            variant_convert(var);
        }
        LOG_CAST_TYPE();
        return var;
    }
    if (numeric_types(typeFrom.get(), type().get()))
    {
        conv_ = CAST_IMPLICT;
        LOG_CAST_TYPE();
        return variant_convert(var);
    }

    RefPtr<PointerType> ptrTo = interface_cast<PointerType>(type());
    RefPtr<PointerType> ptrFrom = interface_cast<PointerType>(typeFrom);

    // converting to reference?
    if (ptrTo.get() && ptrTo->is_reference()
        && (!ptrFrom || !ptrFrom->is_reference()))
    {
        // strip the reference part
        RefPtr<DataType> typeTo = ptrTo->pointed_type();

        conv_ = try_id_and_qualified(typeTo, typeFrom, constCastOK_);

        if (conv_ != CAST_NONE)
        {
            RefPtr<DebugSymbol> sym = var->debug_symbol();
            addr_t addr = 0;
            if (!sym)
            {
                //TODO: DEPRECATE Context::push_temp?
                //addr = context.push_temp(*var);
                addr = context.push_arg(*var);
            }
            else
            {
                addr = sym->addr();
            }
            LOG_CAST_TYPE();

            sym = context.new_temp_ptr(*ptrTo, addr);
            sym->read(&context);
            return new VariantImpl(*sym);
        }
    }
    if (ptrFrom.get() && !variant_true(*var))
    {
        // casting a NULL pointer?
        return variant_convert(var);
    }
    if (try_base_and_derived(context, var, ptrTo.get(), ptrFrom.get()))
    {
        assert(conv_ != CAST_NONE);
        LOG_CAST_TYPE();

        return var;
    }
    // check for ctors and conversion operators
    if (try_conversion(context, *type()))
    {
        conv_ = CAST_USER_CONVERSION;
        return var;
    }
    DEBUG_OUT << "type=" << type()->name() << endl;
    return variant_convert(var);
}


////////////////////////////////////////////////////////////////
CastExpr::Conversion
CastExpr::try_id_and_qualified
(
    const RefPtr<DataType>& typeTo,
    const RefPtr<DataType>& typeFrom,
    bool constCastOK
) const
{
    assert(typeTo.get());
    assert(typeFrom.get());
    TRACE_CAST();

    if (typeTo->is_equal(typeFrom.get()))
    {
        return CAST_IDENTITY;
    }
    // comparison must be symmetrical
    assert(!typeFrom->is_equal(typeTo.get()));

    if (try_qualified(typeTo, typeFrom))
    {
        // the `to' type just adds qualifiers -- we're done
        return CAST_CV_QUALIFIED;
    }

    // is the `to' type stripping cv qualifiers?
    // -- cv-qualified cast in reverse
    if (constCastOK && try_qualified(typeFrom, typeTo))
    {
        return CAST_CONST_CAST;
    }
    return CAST_NONE;
}


////////////////////////////////////////////////////////////////
bool
CastExpr::try_qualified(RefPtr<DataType> typeTo,
                        RefPtr<DataType> typeFrom) const
{
    // pre-conditions
    assert(typeTo.get());
    assert(typeFrom.get());
    assert(!typeTo->is_equal(typeFrom.get())); // handled elsewhere

    if (interface_cast<QualifiedType*>(typeTo.get()))
    {
        typeTo = remove_qualifiers(typeTo);
        assert(typeTo.get());
        TRACE_CAST();

        typeFrom = remove_qualifiers(typeFrom);
        assert(typeFrom.get());
        TRACE_CAST();

        if (typeTo->is_equal(typeFrom.get()))
        {
            // assert symmetry
            assert(typeFrom->is_equal(typeTo.get()));
            return true;
        }
    }

#if DEBUG
    if (typeFrom->is_equal(typeTo.get()))
    {
        clog << "typeFrom=" << typeFrom->name() << endl;
        clog << "typeTo=" << typeTo->name() << endl;
    }
    assert(!typeFrom->is_equal(typeTo.get())); // symmetrical
#endif

    RefPtr<PointerType> ptrFrom = interface_cast<PointerType>(typeFrom);
    RefPtr<PointerType> ptrTo = interface_cast<PointerType>(typeTo);

    if (ptrFrom.get() && ptrTo.get()
        && ptrFrom->is_reference() == ptrTo->is_reference())
    {
        typeTo = ptrTo->pointed_type();
        typeFrom = ptrFrom->pointed_type();
        return try_id_and_qualified(typeTo, typeFrom, false);
    }
    return false;
}


////////////////////////////////////////////////////////////////
bool CastExpr::try_conversion(Context& context, DataType& toType)
{
    assert(!castFrom_->type()->is_equal(&toType));

    TRACE_CAST_(&toType, castFrom_->type());
    RefPtr<CallSetup> setup;
    bool result = false;

     // update the number of successive conversions;
     // get_call_setup (which is invoked from find_conv_ctor()
     // and find_conv_operator()) checks for > 1 user conversions
     auto_ptr<Temporary<size_t> > tmp =
        context.increment_conversion_count();

    if (ClassType* klass = interface_cast<ClassType*>(&toType))
    {
        setup = find_conv_ctor(context, *klass);
    }
    if (!setup)
    {
        assert(castFrom_->type().get());

        if (ClassType* klass =
            interface_cast<ClassType*>(castFrom_->type().get()))
        {
            setup = find_conv_operator(context, *klass, toType);
        }
    }
    if (setup.get())
    {
        result = true;
        DEBUG_OUT << "calling user_conversion\n";
        context.call_function(*setup);
    }
    else
    {
        DEBUG_OUT << "no call setup\n";
    }
    return result;
}


////////////////////////////////////////////////////////////////
//
// todo: is there any way of checking for explicit ctors?
// -- check with David Anderson and DWARF discussion groups
// (AFAIK debug formats do not carry such information)
//
RefPtr<CallSetup> CastExpr::find_conv_ctor(
    Context&    ctxt,
    ClassType&  klass)
{
    // the ctor is expected to have 2 args; notes: implicit params
    // are not handled; first param is the hidden `this' pointer
    ExprList args;

    RefPtr<DebugSymbol> thisPtr(
        make_this_param(*CHKPTR(interp()), klass, args));

    const addr_t addr = thisPtr->addr();
    DataType* const type = thisPtr->type();

    args.push_back(cast_from());
    RefPtr<CallSetup> setup =
        get_ctor_setup(ctxt, *this, klass, addr, type, args);

    return setup;
}


////////////////////////////////////////////////////////////////
// look for a suitable conversion operator
RefPtr<CallSetup>
CastExpr::find_conv_operator(Context&    ctxt,
                             ClassType&  klass,
                             DataType&   type)
{
    RefPtr<CallSetup> setup;

    ExprList args; // the arguments of the conversion operator
    RefPtr<Expr> thisPtr(
        new UnaryExpr(this->interp().get(),
                    UnaryExpr::ADDR,
                    this->cast_from(),
                    UnaryExpr::INTERNAL));
    args.push_back(thisPtr);

    const size_t count = klass.method_count();
    DEBUG_OUT << count << " method(s) in " << klass.name() << endl;

    // iterate over methods and look for conversion operators
    for (size_t i = 0; i != count; ++i)
    {
        const Method* m = CHKPTR(klass.method(i));
        const char* fname = CHKPTR(m->name())->c_str();
        DEBUG_OUT << fname << endl;

        RefPtr<FunType> funType = m->type();
        if (!funType)
        {
            continue;
        }
        // the operator must have exactly one argument (this)
        if (funType->param_count() != 1)
        {
            continue;
        }
        DEBUG_OUT << funType->return_type()->name() << endl;

        if (!equivalent_arg(&type, funType->return_type())
         && !numeric_types(&type, funType->return_type()))
        {
            continue;
        }
        if (strncmp("operator ", fname, 9) == 0)
        {
            // GCC, DWARF debug format
            setup = get_call_setup(ctxt, *this, *funType, 0, 0, &args);
        }
        // GCC 2.95, STABS format : __op<mangled-type>
        else if (strncmp("__op", fname, 4) == 0)
        {
            setup = get_call_setup(ctxt, *this, *funType, 0, m->name(), &args);
        }
        if (setup.get())
        {
            setup->set_fname(m->linkage_name());
            break;
        }
        else
        {
            DEBUG_OUT << fname << ": method not suitable\n";
        }
    }
    return setup;
}


////////////////////////////////////////////////////////////////
bool CastExpr::try_base_and_derived
(
    Context&            context,
    RefPtr<Variant>&    var,
    PointerType*        ptrTo,
    PointerType*        ptrFrom
)
{
    if (!ptrTo)
    {
        // the target expression is not a pointer nor a reference
        return false;
    }
    if (!ptrTo->is_reference() && ptrFrom && ptrFrom->is_reference())
    {
        return false; // casting from reference to pointer
    }
    DebugSymbol* sym = var->debug_symbol();
    if (!sym)
    {
        return false;
    }
    PointerType* refType = 0;
    if (ptrTo->is_reference() && (!ptrFrom || !ptrFrom->is_reference()))
    {
        assert(ptrTo->pointed_type());
        refType = ptrTo;

        if (ptrFrom)
        {
            ptrTo = interface_cast<PointerType*>(ptrTo->pointed_type());
            if (!ptrTo)
            {
                return false;
            }
            // reference to reference is illegal
            assert(!ptrTo->is_reference());
        }
    }

    // casting to pointer-to-class?
    ClassType* klassTo = interface_cast<ClassType*>(ptrTo->pointed_type());
    ClassType* klassFrom = NULL;

    if (!ptrFrom)
    {
        klassFrom = interface_cast<ClassType*>(castFrom_->type().get());
    }
    else
    {
        assert(ptrFrom->is_reference() == ptrTo->is_reference());
        klassFrom = interface_cast<ClassType*>(ptrFrom->pointed_type());
        if (sym->enum_children() == 0)
        {
            sym->read(&context);
        }
        sym = sym->nth_child(0);
        assert(sym);
    }
    if (!klassFrom)
    {
        return false;
    }
    if (klassFrom->has_vtable(context.thread()))
    {
        isDynamic_ = true;
    }
    if (!klassTo)
    {
        return false;
    }

    addr_t addr = 0;
    off_t offset = 0;

    // casting from derived to base?
    if (klassFrom->lookup_base(klassTo->name(), &offset, true)
     || klassFrom->lookup_base(klassTo->unqualified_name(), &offset, true))
    {
        addr = sym->addr();
        if (offset < 0)
        {
            offset = get_vtable_adjustment(*sym->thread(), addr, offset);
        }

        DEBUG_OUT << "offset=" << offset << endl;

        addr += offset;
        conv_ = CAST_DERIVED_TO_BASE;
    }

    if (!addr)
    {
        addr = adjust_base_to_derived(*sym, *klassFrom, *klassTo);
        if (addr)
        {
            conv_ = CAST_BASE_TO_DERIVED;
        }
    }
    if (addr)
    {
        if (refType && refType != ptrTo)
        {
            assert(ptrFrom);
            clog << "FIXME: " << __FILE__ << ":" << __LINE__ << endl;
        }
        // make pointer to the adjusted address
        RefPtr<DebugSymbol> ptr = context.new_temp_ptr(*ptrTo, addr);
        ptr->read(&context);

        var = new VariantImpl(*ptr);
        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////
void CastExpr::throw_invalid_cast(const string& castType)
{
    string msg = "invalid " + castType + " from ";

    CHKPTR(cast_from());
    CHKPTR(cast_from()->type());
    CHKPTR(cast_from()->type()->name());

    msg += cast_from()->type()->name()->c_str();
    msg += " to ";
    msg += type()->name()->c_str();

    throw EvalError(msg);
}


////////////////////////////////////////////////////////////////
RefPtr<Variant> CastExpr::variant_convert(RefPtr<Variant>& v) const
{
    // pre-conditions
    assert(cast_from()->type().get());
    assert(type().get());
    assert(type() == cast_to()->type());

    ::variant_convert(v, *type());
    if (!v || v->size() == 0)
    {
        ostringstream err;
        err << "An error occurred while casting ";
        err << cast_from()->type()->name() << " to ";
        err << type()->name();

        throw runtime_error(err.str());
    }
    return v;
}


////////////////////////////////////////////////////////////////
StaticCast::StaticCast(RefPtr<Expr> castTo, RefPtr<Expr> castFrom)
    : CastExpr(castTo, castFrom, false)
{
}


////////////////////////////////////////////////////////////////
void StaticCast::throw_invalid_cast(const std::string& msg)
{
    CastExpr::throw_invalid_cast(
        "cast, try again using C-style cast or reinterpret_cast: "
        + msg);
}


/*
static bool is_boolean(const DataType& type)
{
    bool result = type.is_fundamental()
        && CHKPTR(type.name())->is_equal("bool");
    return result;
}
*/


////////////////////////////////////////////////////////////////
RefPtr<Variant> StaticCast::eval_impl(Context& context)
{
    RefPtr<Variant> var = CastExpr::eval_impl(context);
    if (!var)
    {
        throw logic_error("cast expression yielded null");
    }
    // enforce static_cast rules
    RefPtr<DataType> fromType = cast_from()->type();

    assert(fromType.get()); // CastExpr::eval_impl contract
    assert(type().get());   // idem

    bool valid = false;

    if (is_derived_to_base()
        || is_base_to_derived()
        || is_cv_qualified()
        || is_identity()
        || is_implicit()
        || is_user_conversion())
    {
        valid = true;
    }
    else if (interface_cast<IntType*>(fromType.get()))
    {
        valid = interface_cast<IntType*>(type().get())
             || interface_cast<FloatType*>(type().get())
             || type()->name()->is_equal("bool");
    }
    else if (interface_cast<FloatType*>(fromType.get()))
    {
        valid = interface_cast<IntType*>(type().get())
             || interface_cast<FloatType*>(type().get())
             || type()->name()->is_equal("bool");
    }
    else if (interface_cast<EnumType*>(type().get()))
    {
        valid = interface_cast<IntType*>(type().get());
    }
    if (!valid)
    {
        throw_invalid_cast("static_cast");
    }
    return var;
}


////////////////////////////////////////////////////////////////
ImplicitCast::ImplicitCast
(
    RefPtr<Expr> castTo,
    RefPtr<Expr> castFrom
)
    : StaticCast(castTo, castFrom)
{
}


////////////////////////////////////////////////////////////////
RefPtr<Variant> ImplicitCast::eval_impl(Context& context)
{
    RefPtr<Variant> var = StaticCast::eval_impl(context);

    if (!var)
    {
        throw logic_error("cast expression yielded null");
    }
    // base-to-derived is ok in static_cast, but
    // it is not allowed in an implicit cast
    if (is_base_to_derived())
    {
        throw_invalid_cast("implicit cast");
    }
    return var;
}


////////////////////////////////////////////////////////////////
/*
void ImplicitCast::throw_invalid_cast(const string& type)
{
    CastExpr::throw_invalid_cast(type);
}
*/


////////////////////////////////////////////////////////////////
ConstCast::ConstCast(RefPtr<Expr> to, RefPtr<Expr> from)
    : CastExpr(to, from)
{
}


////////////////////////////////////////////////////////////////
RefPtr<Variant> ConstCast::eval_impl(Context& context)
{
    RefPtr<Variant> v = CastExpr::eval_impl(context);

    if (!is_identity() && !is_const_cast())
    {
        throw_invalid_cast(_name());
    }
    return v;
}


////////////////////////////////////////////////////////////////
DynamicCast::DynamicCast(RefPtr<Expr> to, RefPtr<Expr> from)
    : CastExpr(to, from)
{
}


////////////////////////////////////////////////////////////////
RefPtr<Variant> DynamicCast::eval_impl(Context& context)
{
    RefPtr<Variant> v = eval_sub_expressions(context);

    RefPtr<PointerType> ptrTo = interface_cast<PointerType>(type());
    if (!ptrTo)
    {
        throw_invalid_cast(_name()
            + string(" (target is not pointer or reference)"));
    }

    RefPtr<PointerType> ptrFrom = interface_cast<PointerType>(cast_from()->type());
    if (ptrTo->is_reference())
    {
        if (!ptrFrom || !ptrFrom->is_reference())
        {
            throw_invalid_cast(_name() + string(" (source is not a reference)"));
        }
    }
    else
    {
        if (!ptrFrom || ptrFrom->is_reference())
        {
            throw_invalid_cast(_name() + string(" (source is not a pointer)"));
        }
    }
    v = CastExpr::eval_impl(context);

    if (!is_identity() && !is_dynamic())
    {
        throw_invalid_cast(_name());
    }
    if (!is_identity() && !is_base_to_derived())
    {
        if (ptrTo->is_reference())
        {
            throw EvalError("bad_cast");
        }
        RefPtr<DebugSymbol> sym = context.new_temp_ptr(*ptrTo, 0);
        v = new VariantImpl(*sym);
    }
    return v;
}


////////////////////////////////////////////////////////////////
ReinterpretCast::ReinterpretCast(RefPtr<Expr> to, RefPtr<Expr> from)
    : CastExpr(to, from)
{
}


////////////////////////////////////////////////////////////////
RefPtr<Variant> ReinterpretCast::eval_impl(Context& context)
{
    RefPtr<Variant> v = eval_sub_expressions(context);
    assert(v.get()); // post-condition
    assert(type() == cast_to()->type()); // post-condition

    return variant_convert(v);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
