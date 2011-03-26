//
// $Id: postfix_expr.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <assert.h>
#include <stdexcept>
#include <string>
#include <boost/tokenizer.hpp>
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "zdk/container.h"
#include "zdk/types.h"
#include "zdk/variant_util.h"
#include "typez/public/debug_symbol_impl.h"
#include "typez/public/lookup_child.h"
#include "call_setup.h"
#include "cast_expr.h"
#include "collect_symbol.h"
#include "debug_out.h"
#include "errors.h"
#include "eval_inc_dec.h"
#include "ident.h"
#include "interp.h"
#include "lookup_methods.h"
#include "postfix_expr.h"
#include "rtti_disabler.h"
#include "slicer.h"
#include "type_spec.h"
#include "unary_expr.h"
#include "variant_assign.h"
#include "variant_impl.h"


using namespace std;



/**
 * Helper used for evaluating the '.' and '->' operators
 */
static ClassType&
get_class_type(DataType* type, const string& member)
{
    ClassType* klass = interface_cast<ClassType*>(type);
    if (!klass)
    {
        string err("requesting '" + member + "' in non-aggregate ");
        err += CHKPTR(type->name())->c_str();

        throw EvalError(err);
    }
    return *klass;
}


PostfixExpr::PostfixExpr
(
    Interp*             interp,
    Operator            oper,
    const RefPtr<Expr>& expr,
    const RefPtr<Expr>& param
)
  : Expr(interp)
  , oper_(oper)
  , expr_(expr)
  , param_(param)
  , isMacroCall_(false)
{
    assert(expr_.get());

    if (!param_)
    {
        assert(oper_ == FCALL || oper_ == INC || oper_ == DEC);
    }
    if (oper_ == MEMBER
     || oper_ == POINTER
     || oper_ == VPOINTER
     || oper_ == PTR_TO_MEMBER)
    {
        assert(param_);
        ident_ = &interface_cast<Ident&>(*param_);
    }
}


PostfixExpr::~PostfixExpr() throw()
{
}



RefPtr<Variant> PostfixExpr::eval_impl(Context& context)
{
    auto_ptr<RTTI_Disabler> rttiDisable;
    if (oper_ != VPOINTER)
    {
        // all but operator => disable auto-RTTI because we don't
        // want a pointer to the most-derived object to be returned;
        // operator => is my own extension to the language, useful
        // for testing
        rttiDisable.reset(new RTTI_Disabler(context.type_system()));
    }
    context.set_macro_helper(this);
    RefPtr<Variant> lval = expr_->eval(context);

    if (!lval)
    {
        throw logic_error("base expression yielded null");
    }
    if (isMacroCall_)
    {
        set_result(lval);
        assert(type() == expr_->type());
        return lval;
    }
    // type should be known at this point;
    // if not, then it's a programming mistake somewhere
    if (lval->type_tag() == Variant::VT_NONE || !expr_->type())
    {
        throw logic_error("unknown argument type");
    }
    RefPtr<Variant> result;

    if ((oper_ != POINTER) && (oper_ != VPOINTER))
    {
        // handle pointer operator separately
        RefPtr<CallSetup> op = get_overloaded_operator(context);
        if (op.get())
        {
            dump_args(context, op->args());
            initiate_fun_call(context, *op);
            return result;
        }
    }
    switch (oper_)
    {
    case ARRAY:
        result = eval_array(context, *lval);
        break;

    case ARANGE:
        result = eval_array_range(context, *lval);
        break;

    case FCALL:
        result = eval_function_call(context, *lval);
        break;

    case MEMBER:
        result = eval_member(context, *lval);
        break;

    case PTR_TO_MEMBER:
        result = eval_ptr_to_member(context, *lval);
        break;

    case POINTER:
    case VPOINTER:
        {
            RefPtr<DebugSymbol> obj = eval_pointer(context, *lval);
            if (obj.get())
            {
                return eval_member(context, *obj, ident_->name(), true);
            }
        }
        break;

    case INC:
    case DEC:
        // the resulting expression is of the same type as the arg
        set_type(expr_->type());
        eval_inc_dec(context, *expr_->type(), *lval, (oper_ == INC));

        // copy the old value, but the symbol part,
        // since the result is a non l-value
        result = new VariantImpl;
        result->copy(lval.get(), false);
        break;
    }
    return result;
}


RefPtr<Variant>
PostfixExpr::eval_member(Context&        context,
                         DebugSymbol&    object,
                         const string&   memberName,
                         bool            isPointerOrRef)
{
    DataType* type = object.type();
    if (!type)
    {
        throw logic_error("null object type");
    }

    // is the base expression (left of the dot) a reference?
    if (PointerType* ref = interface_cast<PointerType*>(type))
    {
        if (isPointerOrRef || ref->is_reference())
        {
            // pointer to reference is illegal in C++
            assert(isPointerOrRef || oper_ != POINTER);
            type = ref->pointed_type();
        }
        isPointerOrRef = true;
    }
    if (DebugSymbol* parent = object.parent())
    {
        if (PointerType* ref = interface_cast<PointerType*>(parent->type()))
        {
            if (ref->is_reference())
            {
                isPointerOrRef = true;
            }
        }
    }

    RefPtr<DebugSymbol> sym = lookup_child(object, memberName.c_str());
    if (!sym)
    {
        // lookup methods polymorphically if pointer or reference
        const addr_t addr = isPointerOrRef ? object.addr() : 0;
        DEBUG_OUT << (void*)addr << endl;

        ClassType& klass = get_class_type(object.type(), memberName);
        sym = lookup_methods(context, klass, ident_->name(), addr);
    }
    if (!sym)
    {
        string err = CHKPTR(type->name())->c_str();
        err += " has no member named '" + memberName + "'";
        throw EvalError(err);
    }
    assert(sym->type());
    set_type(sym->type());

    // test for an overloaded function -- lookup_method may return
    // a FunType symbol that has the overloads as its children, and
    // a zero address; we don't want to read it, since the implementation
    // of DebugSymbol::read() resets the children.
    if (sym->addr() || sym->enum_children() == 0)
    {
        sym->read(&context);
    }
    assert(sym->addr() || sym->is_constant() || sym->enum_children());
    return new VariantImpl(*sym);
}


RefPtr<Variant> PostfixExpr::eval_member(Context& ctxt, const Variant& lval)
{
    RefPtr<DebugSymbol> obj = lval.debug_symbol();
    if (!obj)
    {
        throw EvalError("operator '.' with NULL object");
    }
    assert(ident_);
    return eval_member(ctxt, *obj, ident_->name());
}


RefPtr<Variant>
PostfixExpr::eval_ptr_to_member(Context& ctxt, const Variant& lval)
{
    RefPtr<DebugSymbol> obj = lval.debug_symbol();
    if (!obj)
    {
        throw EvalError("NULL base object");
    }
    if (!obj->type())
    {
        throw EvalError("NULL base type");
    }
    if (interface_cast<PointerType*>(obj->type()))
    {
        obj->read(&ctxt);
        obj = obj->nth_child(0);
        if (!obj)
        {
            throw EvalError("error dereferencing pointer");
        }
    }
    RefPtr<DebugSymbol> sym = ctxt.lookup_debug_symbol(CHKPTR(ident_)->name().c_str());
    if (!sym)
    {
        throw EvalError("symbol not found: " + ident_->name());
    }
    RefPtr<PtrToMemberType> ptrMem = interface_cast<PtrToMemberType*>(sym->type());
    if (!ptrMem)
    {
        throw EvalError(ident_->name() + " is not a pointer to member");
    }
    CHKPTR(ptrMem->pointed_type());

    if (!obj->type()->is_equal(CHKPTR(ptrMem->base_type())))
    {
        throw EvalError(string("pointer to member type ")
                        + CHKPTR(ptrMem->pointed_type()->name())->c_str()
                        + " incompatible with object type "
                        + CHKPTR(obj->type()->name())->c_str());
    }
    set_type(ptrMem->pointed_type());
    DebugSymbolImpl& impl = interface_cast<DebugSymbolImpl&>(*sym);
    impl.read(&ctxt);
    if (SharedString* val = impl.value())
    {
        impl.set_addr(strtoul(val->c_str(), 0, 0) + obj->addr());
        impl.set_type(*type());
        return new VariantImpl(impl);
    }
    return new VariantImpl;
}


/**
 * This is a separate function so that it can be used with both
 * "normal" pointers and overloaded -> operators; with the latter,
 * this function is called asynchronously after the operator call
 * returns.
 */
RefPtr<DebugSymbol> PostfixExpr::eval_pointer
(
    Context& ctxt,
    const RefPtr<Expr>& expr,
    const RefPtr<Variant>& var
)
{
    if (var->type_tag() != Variant::VT_POINTER)
    {
        throw EvalError("base operand of -> is not a pointer");
    }
    PointerType& ptrType = interface_cast<PointerType&>(*expr->type());
    if (ptrType.is_reference())
    {
        throw EvalError("base operand of -> is a reference");
    }
    RefPtr<DebugSymbol> symPtr = var->debug_symbol();
    if (!symPtr)
    {
        assert(ptrType.is_equal(expr_->type().get()));
        symPtr = ctxt.new_temp_ptr(ptrType, var->pointer());
    }

    if (!symPtr)
    {
        // this should only happen in the unit tests, because
        // the TestContext returns null temp pointers
        throw EvalError("base operand of -> is not a lvalue");
    }

    if (symPtr->addr() == 0 && !symPtr->value())
    {
        throw EvalError("dereferencing NULL pointer");
    }
    symPtr->read(&ctxt);

    // the pointer object is expected to have one child
    // (the debug symbol corresponding to the pointee)
    if (symPtr->enum_children() == 0)
    {
        string typeName = symPtr->type()->name()->c_str();
        throw logic_error(typeName + ": could not get pointed object");
    }
    RefPtr<DebugSymbol> obj = symPtr->nth_child(0);

    return CHKPTR(obj);
}



RefPtr<DebugSymbol>
PostfixExpr::eval_pointer(Context& ctxt, const Variant& lval)
{
    assert(ident_.get());

    // first, evaluate the expression left of '->', which
    // should yield a either pointer-type, or a class instance
    RefPtr<Variant> tmp = expr_->eval(ctxt);
    if (!tmp)
    {
        throw logic_error("base operand of -> yielded null");
    }
    if (!expr_->type())
    {
        throw logic_error("null expression type");
    }
    check_for_operator_arrow(ctxt, tmp);
    return eval_pointer(ctxt, expr_, tmp);
}



static void
enforce_integer_type(const Variant& var, const Expr& param)
{
    if (!is_integer(var))
    {
        string err("invalid type ");

        if (RefPtr<DataType> type = param.type())
        {
            err += CHKPTR(type->name())->c_str();
            err += " ";
        }
        err += "for array subscript";
        throw EvalError(err);
    }
}


static uint64_t
array_size(DebugSymbol* array, const ArrayType* arrayType)
{
    uint64_t upper = numeric_limits<int64_t>::max();

    if (const DynamicArrayType* dynArray =
            interface_cast<const DynamicArrayType*>(arrayType))
    {
        upper = dynArray->count(array);
    }
    else if (arrayType)
    {
        // array of fixed size
        upper = arrayType->elem_count();
    }
    return upper;
}


/**
 * Helper function, extract index value from variant, and
 * check bounds
 */
static uint64_t
get_index(const Variant&    index,
          DebugSymbol*      array = 0,
          const ArrayType*  arrayType = 0)
{
    uint64_t result = 0;
    const uint64_t upper = array_size(array, arrayType);

    if (index.type_tag() == Variant::VT_UINT64)
    {
        if (index.uint64() >= upper)
        {
            throw range_error("array subscript is out of range");
        }
        result = index.uint64();
    }
    else
    {
        //
        // no negative indices allowed in C/C++
        //
        if ((index.int64() < 0) ||
            (static_cast<uint64_t>(index.int64()) >= upper)
           )
        {
            throw range_error("array subscript is out of range");
        }
        result = index.int64();
    }
    return result;
}



RefPtr<Variant>
PostfixExpr::eval_array_range(Context& ctxt, const Variant& lval)
{
    DataType* type = CHKPTR(expr_)->type().get();
    ArrayType* arrayType = interface_cast<ArrayType*>(type);
    if (!arrayType)
    {
        throw EvalError("base operand of [:] is not an array");
    }

    // must have an argument list by grammar rules
    ExprList args = interface_cast<ArgumentList&>(*CHKPTR(param_)).args();
    if (args.size() != 2)
    {
        throw logic_error("unexpected range specification");
    }

    RefPtr<DebugSymbol> sym = lval.debug_symbol();

    uint64_t low = 0, high = 0;

    if (args[0])
    {
        low = get_index(*args[0]->eval(ctxt), sym.get(), arrayType);
    }
    if (args[1])
    {
        high = get_index(*args[1]->eval(ctxt), sym.get(), arrayType);
        if (high)
        {
            --high;
        }
    }
    if (low >= high)
    {
        high = array_size(sym.get(), arrayType);
        if (high)
        {
            --high; // the highest valid index in the array
        }
    }
    RefPtr<DataType> elemType = CHKPTR(arrayType->elem_type());
    RefPtr<DataType> resultType =
        ctxt.type_system().get_array_type(low, high, elemType.get());

    addr_t addr = arrayType->first_elem_addr(sym.get());

    if (addr)
    {
        addr += elemType->size() * low;
    }
    RefPtr<DebugSymbol> newSym =
        ctxt.type_system().create_debug_symbol(
                sym->reader(),
                sym->thread(),
                resultType.get(),
                sym->name(),
                addr,
                NULL,   //sym->decl_file(),
                0,      //sym->decl_line(),
                sym->is_return_value());
    newSym->read(&ctxt);

    if (addr == 0)
    {
        // support synthesised arrays
        newSym->read(&ctxt);
        Slicer slicer(sym);
        slicer.run(low, high, newSym);

        interface_cast<DebugSymbolImpl&>(*newSym).set_value(sym->value());
    }

    set_type(resultType);
    return new VariantImpl(*newSym);
}


RefPtr<Variant>
PostfixExpr::eval_array(Context& ctxt, const Variant& lval)
{
    if ((lval.type_tag() != Variant::VT_ARRAY)
     && (lval.type_tag() != Variant::VT_POINTER))
    {
        throw EvalError("base operand of [] is not pointer or array");
    }

    if (!param_)
    {
        // at this point a non-null subscript expression is expected
        throw logic_error("null subscript expression");
    }
    RefPtr<Variant> rval = param_->eval(ctxt);
    // expect the evalution to succeed and to yield an integer result
    if (!rval)
    {
        throw logic_error("subscript expression yielded null");
    }
    enforce_integer_type(*rval, *param_);

    RefPtr<DebugSymbol> sym;

    DataType* type = expr_->type().get();
    if (PointerType* ptrType = interface_cast<PointerType*>(type))
    {
        if (ptrType->is_reference())
        {
            throw EvalError("base operand of [] is not an array");
        }
        RefPtr<DebugSymbol> ptr = CHKPTR(lval.debug_symbol());
        ptr->read(&ctxt);

        if (RefPtr<DebugSymbolImpl> elem =
            interface_cast<DebugSymbolImpl*>(ptr->nth_child(0)))
        {
            const size_t elemSize = CHKPTR(elem->type())->size();
            const addr_t addr = elem->addr();

            elem->set_addr(addr + elemSize * get_index(*rval));

            elem->read(&ctxt);
            sym = elem;
        }
    }
    else
    {
        ArrayType* arrayType = interface_cast<ArrayType*>(type);
        if (!arrayType)
        {
            if (interface_cast<AssociativeContainerType*>(type))
            {
                throw runtime_error("subscript not supported in associative containers");
            }
        #if DEBUG
            clog << type->name() << ": not an array\n";
        #endif
            throw EvalError("base operand of [] is not an array");
        }

        uint64_t index = get_index(*rval, lval.debug_symbol(), arrayType);

        if (!lval.debug_symbol())
        {
            throw logic_error("null array symbol");
        }
        lval.debug_symbol()->read(&ctxt);
        sym = lval.debug_symbol()->nth_child(index);
    }
    set_type(sym->type());

    return new VariantImpl(*sym);
}



RefPtr<Variant>
PostfixExpr::eval_function_call(Context& ctxt, Variant& lval)
{
    ctxt.reset_frame_setup();
    if (DebugSymbol* fun = lval.debug_symbol())
    {
        assert (!interface_cast<MacroType*>(fun->type()));
        // function calls are evaluated asynchronously,
        // therefore there is no return value here
        eval_function_call(ctxt, *fun);
        DEBUG_OUT << fun->name() << endl;
    }
    else
    {
        throw logic_error("null function symbol in function call");
    }
    return RefPtr<Variant>();
}


RefPtr<CallSetup>
PostfixExpr::get_overloaded_operator(Context& ctxt, bool strict)
{
    RefPtr<CallSetup> result;

    RefPtr<ClassType> klass = interface_cast<ClassType>(expr_->type());
    if (klass.get())
    {
        string opname = "operator";
        switch (oper_)
        {
        case ARRAY: opname += "[]"; break;
        case FCALL: opname += "()"; break;
        case POINTER: opname += "->"; break;
        case INC: opname+= "++"; break;
        case DEC: opname+= "--"; break;
        default: return NULL;
        }

        RefPtr<DebugSymbol> fun = lookup_methods(ctxt, *klass, opname);
        if (!fun)
        {
            if (!strict)
            {
                return result;
            }
            const char* kname = CHKPTR(klass->name())->c_str();
            throw EvalError(opname + " not found in: " + kname);
        }

        ExprList args;

        // push the hidden "this" parameter onto the stack
        args.push_back(
            new UnaryExpr(interp().get(),
                          UnaryExpr::ADDR,
                          expr_,
                          UnaryExpr::INTERNAL));
        switch (oper_)
        {
        case INC:
        case DEC:
            {
                // postfix operators take a dummy int parameter
                RefPtr<DataType> intType(ctxt.get_int_type());
                RefPtr<DebugSymbol> dummy(ctxt.new_const(*intType, "0xdead"));
                args.push_back(new Ident(interp().get(), "", dummy));
            }
            break;

        case ARRAY:
            assert(param_.get());
            args.push_back(param_);
            break;

        case FCALL:
            {
                ExprList& param =
                    interface_cast<ArgumentList&>(*param_).args();
                args.insert(args.end(), param.begin(), param.end());
            }
            break;

        default:
            break;
        }

        result = get_call_setup(ctxt, *this, fun.get(), &args);
        if (!result)
        {
            no_matching_func(ctxt, *fun, &args); // emit error
        }
        else if (oper_ == INC || oper_ == DEC)
        {
            // int argument is used for signature only,
            // not passed to operator
            if (result->args().size() == 2)
            {
                result->args().pop_back();
            }
        }
    }
    return result;
}


void
PostfixExpr::eval_function_call(Context& ctxt, DebugSymbol& fun)
{
#if !defined(__i386__) && !defined(__x86_64__) // fixme
    throw runtime_error(__func__ + string(": not implemented on this platform"));
#endif
    ctxt.reset_frame_setup();

// gcc 2.95 bug workaround
#if !defined(DEBUG) && (__GNUC__ < 3)
  // I don't know why, but this somehow prevents the 2.95 compiler
  // from crashing, when in release (optimizing) mode
  try
  {
#endif

    ExprList args;
    if (param_) // function call has arguments?
    {
        args = interface_cast<ArgumentList&>(*param_).args();
    }

    RefPtr<CallSetup> setup;
    Method* method = interface_cast<Method*>(&fun);

    DEBUG_OUT << fun.name() << " *** method=" << method << " ***\n";

    if (method && !method->is_static())
    {
        DEBUG_OUT << "trying member functions\n";
        setup = get_method(ctxt, fun, &args);
    }
    else
    {
        setup = get_call_setup(ctxt, *this, &fun, &args);
        // <workaround bug in STABS reader>
        if (!setup && method)
        {
            setup = get_method(ctxt, fun, &args);
        }
        // </workaround>
    }
    if (!setup)
    {
        no_matching_func(ctxt, fun, &args);
    }
    else
    {
        assert(setup->reader() == fun.reader()); // post-condition
    }

    initiate_fun_call(ctxt, *setup);

// gcc 2.95 bug workaround
#if !defined(DEBUG) && (__GNUC__ < 3)
  }
  catch (...)
  {
    throw;
  }
#endif
}


RefPtr<CallSetup>
PostfixExpr::get_method(Context& ctxt,
                        DebugSymbol& fun,
                        ExprList* args)
{
    // add the hidden `this' parameter to the arg list
    RefPtr<Expr> thisPtr;

    if (PostfixExpr* pfix = interface_cast<PostfixExpr*>(expr_.get()))
    {
        // the hidden THIS parameter
        switch (pfix->oper())
        {
        case POINTER:
        case VPOINTER:
            if (!pfix->ptr_)
            {
                thisPtr = pfix->expr_;
            }
            else
            {
                thisPtr = pfix->ptr_;
            }
            break;

        case MEMBER:
            thisPtr = new UnaryExpr(this->interp().get(),
                                    UnaryExpr::ADDR,
                                    pfix->expr_,
                                    UnaryExpr::INTERNAL);
        default:
            break;
        }
    }
    RefPtr<CallSetup> setup;

    if (thisPtr)
    {
        if (!param_)
        {
            RefPtr<ArgumentList> param =
                new ArgumentList(interp().get(), thisPtr);
            param_ = param;
            args = &param->args();
        }
        else
        {
            assert(args);
            args->insert(args->begin(), thisPtr);
        }
        setup = get_call_setup(ctxt, *this, &fun, args);
    }
    return setup;
}


void
PostfixExpr::initiate_fun_call(Context& ctxt, CallSetup& setup)
{
    set_strict_type(setup.is_return_type_strict());
    
    if (Thread* thread = ctxt.thread())
    {
        if (Debugger* dbg = thread->debugger())
        {
            dbg->set_breakpoint_at_throw(thread, false);
        }
    }
    // Context::call_function does not return, so if there are
    // any other things to setup, here is the place to do it
    // ...

    ctxt.call_function(setup);
}


void
PostfixExpr::map_arguments(const string& params, ArgMap& argMap)
{
    typedef boost::char_separator<char> Delim;
    typedef boost::tokenizer<Delim> Tokenizer;

    // tokenize macro arguments and map them to the
    // expressions that are then passed as arguments
    // to the call
    Delim delimiters(", )");
    Tokenizer tok(params, delimiters);
    ExprList args;
    if (Expr* param = param_.get()) // the call has arguments?
    {
        ArgumentList& alist = interface_cast<ArgumentList&>(*param);
        args = alist.args();
    }

    Tokenizer::const_iterator i = tok.begin();
    const Tokenizer::const_iterator end = tok.end();
    ExprList::const_iterator j = args.begin();
    for (; i != end; ++i, ++j)
    {
        if (j == args.end())
        {
            throw runtime_error("macro argument missing");
        }
        argMap.insert(make_pair(*i, *j));
    }
    if (j != args.end())
    {
        throw runtime_error("too many macro arguments");
    }
    isMacroCall_ = true;
}


class ZDK_LOCAL PointerExprHelper : public PostfixExpr
{
    RefPtr<PostfixExpr> pfix_;

public:
    PointerExprHelper(Interp* in, const RefPtr<PostfixExpr>& expr)
    : PostfixExpr(in, POINTER, expr, expr->param())
    , pfix_(expr)
    { }

private:
    virtual void set_result(const RefPtr<Variant>& var)
    {
        PostfixExpr::set_result(var);
        pfix_->set_result(var);
    }

    virtual void set_ptr(const RefPtr<Expr>& ptr)
    {
        PostfixExpr::set_ptr(ptr);
        pfix_->set_ptr(ptr);
    }

    RefPtr<Variant> eval_impl(Context& ctxt)
    {
        RefPtr<Variant> var(expr()->value());
        check_for_operator_arrow(ctxt, var);

        RefPtr<DebugSymbol> sym(pfix_->eval_pointer(ctxt, pfix_, var));

        DEBUG_OUT << "sym=" << hex << sym->addr() << dec << endl;
        RefPtr<Interp> interp = expr()->interp();
        RefPtr<Expr> ptr(expr()->clone(interp.get()));
        ptr->set_result(expr()->value());
        pfix_->set_ptr(ptr);

        if (sym.get())
        {
            const string name = CHKPTR(pfix_->ident())->name();
            var = pfix_->eval_member(ctxt, *sym, name, true);

            pfix_->set_result(var);
            DEBUG_OUT << "done: " << pfix_->type()->name() << endl;
        }
        return var;
    }
};


/**
 * @note return void since initiate_fun_call never returns
 */
void
PostfixExpr::check_for_operator_arrow(Context& ctxt, RefPtr<Variant> tmp)
{
    if (tmp->type_tag() == Variant::VT_OBJECT)
    {
        DEBUG_OUT << expr_->type()->name() << endl;
        RefPtr<CallSetup> setup(get_overloaded_operator(ctxt, true));
        if (setup.get())
        {
            DEBUG_OUT << "overloaded operator->()\n";

            // push the helper on the stack, so that Interp::resume()
            // will pick it up, and the evaluation of the pointer
            // expression continues
            RefPtr<Expr> expr(new PointerExprHelper(interp().get(), this));
            ctxt.push(expr);

            initiate_fun_call(ctxt, *setup);
        }
    }
}


RefPtr<Expr> PostfixExpr::clone(Interp* in) const
{
    DEBUG_OUT << "interp=" << in << endl;

    RefPtr<Expr> param(param_ ? param_->clone(in) : 0);
    return new PostfixExpr(in, oper_, expr_->clone(in), param);
}


ArgumentList::ArgumentList(Interp* interp, const RefPtr<Expr>& expr)
    : Expr(interp)
{
    args_.push_back(expr);
}


ArgumentList::~ArgumentList() throw()
{
}


RefPtr<Variant> ArgumentList::eval_impl(Context& context)
{
    // each argument should be evaluated separately
    throw logic_error("unexpected request to evaluate arg list");

    return RefPtr<Variant>(); // keep compiler happy
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
