//
// $Id: call_setup.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/types.h"
#include "zdk/type_system.h"
#include "zdk/variant_util.h"
#include "zdk/zero.h" // for Runnable, Thread
#include "dharma/variant_impl.h"
#include "generic/temporary.h"
#include "typez/public/remove_qual.h"
#include "alt_events.h"
#include "call_setup.h"
#include "cast_expr.h"
#include "constant.h"
#include "context.h"
#include "debug_out.h"
#include "errors.h"
#include "ident.h"
#include "interp.h"
#include "postfix_expr.h"
#include "type_spec.h"
#include "unary_expr.h"

using namespace std;


CallSetup::CallSetup
(
    Expr* expr,
    const ExprList& args,
    const FunType& proto,
    size_t conversionCount
)
    : addr_(0)
    , retAddr_(0)
    , args_(args)
    , expr_(expr)
    , reader_(0)
    , proto_(proto)
    , conversions_(conversionCount)
{
    assert(expr);
}


CallSetup::CallSetup(Expr* expr, const FunType& proto)
    : addr_(0)
    , retAddr_(0)
    , expr_(expr)
    , reader_(0)
    , proto_(proto)
{
}


CallSetup::~CallSetup() throw()
{
}


/**
 * workaround for debug formats (STAB) that do not explicitely
 * specify whether the function takes a variable argument list
 */
static bool
var_args(const FunType& fun, const SharedString* name)
{
    bool result = fun.has_variable_args()
        || (name && strstr(name->c_str(), "..."));
    return result;
}


static void eval_arg(Context& ctxt, const RefPtr<Expr>& arg)
{
    assert(arg);

    RefPtr<Variant> v = arg->eval(ctxt);
    if (!v)
    {
        throw logic_error("null argument");
    }
    if (!arg->type())
    {
        throw logic_error("null argument expression type");
    }
}


/**
 * @return true if argType can be used where an argument of
 * parmType is expected;
 * paramType is what the function prototype specifies,
 * argType is the type of the argument passed in to the
 * function call
 */
bool
equivalent_arg(RefPtr<DataType> paramType,  // expected type
               const DataType* argType,     // actual type
               int argIndex,
               DebugSymbol* fun,
               size_t* conversions          // count successive conversions
              )
{
    assert(paramType);
    if (!argType)
    {
        return false;
    }
    paramType = remove_qualifiers(paramType);
    if (paramType->is_equal(argType))
    {
        //assert(argType->is_equal(paramType.get()));
        return true;
    }
    if (const PointerType* pt2 = interface_cast<const PointerType*>(argType))
    {
        RefPtr<PointerType> pt1 = interface_cast<PointerType>(paramType);
        if (pt1 &&
            (pt1->is_reference() == pt2->is_reference()))
        {
            paramType = pt1->pointed_type();
            argType = pt2->pointed_type();
            return equivalent_arg(paramType, argType, argIndex, fun, conversions);
        }
        if (pt2->is_reference())
        {
            return paramType->is_equal(pt2->pointed_type());
        }
    }
    else if (const ClassType* ct2 = interface_cast<const ClassType*>(argType))
    {
        RefPtr<ClassType> ct1 = interface_cast<ClassType>(paramType);
        if (ct1)
        {
            // a derived instance can be passed where base is expected
            if (ct2->lookup_base(ct1->name(), NULL, true))
            {
                if (conversions)
                {
                    ++*conversions;
                }
                return true;
            }
            if (argIndex == 0)
            {
                // a base can be passed where derived is expected
                // as the THIS parameter of a virtual method
                if (Method* method = interface_cast<Method*>(fun))
                {
                    if (method->is_virtual() &&
                        ct1->lookup_base(ct2->name(), NULL, true))
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


static RefPtr<DebugSymbol>
make_pointer(Context& ctxt, ClassType& klass, addr_t addr)
{
    RefPtr<PointerType> pt = ctxt.type_system().get_pointer_type(&klass);
    assert(pt);

    RefPtr<DebugSymbol> sym = ctxt.new_temp_ptr(*pt, addr);

    assert(sym->type() == pt.get());
    assert(sym->addr() == addr);

    // hack: reading the value has the side effect of fully
    // discovering the class type (deep-adapting is done in the
    // DWARF case, see class_type_proxy.cpp under plugin/dwarf)
    sym->read(&ctxt);

    // read pointed object
    if (sym->enum_children())
    {
        sym->nth_child(0)->read(&ctxt);
    }
    return sym;
}


/**
 * Generate copy constructor call
 */
static RefPtr<Expr> make_copy_ctor_expr(Expr& expr)
{
    Interp& interp = *CHKPTR(expr.interp());
    RefPtr<DebugSymbol> sym = CHKPTR(expr.value())->debug_symbol();

    ClassType& klass = interface_cast<ClassType&>(*CHKPTR(sym->type()));

    ExprList args;
    RefPtr<DebugSymbol> thisPtr(make_this_param(interp, klass, args));

    RefPtr<Expr> rhs = new UnaryExpr(&interp,
                                     UnaryExpr::ADDR,
                                     &expr,
                                     UnaryExpr::INTERNAL);

    args.push_back(rhs);

    Context& ctxt = interp.context();
    RefPtr<CallSetup> setup = get_ctor_setup(ctxt,
                                             expr,
                                             klass,
                                             thisPtr->addr(),
                                             thisPtr->type(),
                                             args);
    RefPtr<Expr> ctor;

    if (setup)
    {
        DEBUG_OUT << "inserting copy ctor call" << endl;

        assert(setup->fname());
        const char* ctorName = setup->fname()->c_str();
        RefPtr<Expr> ident(new Ident(&interp, ctorName, true));
        ctor = new PostfixExpr(&interp,
                                PostfixExpr::FCALL,
                                ident,
                                &expr);
    }
    ctxt.reset_frame_setup();
    return ctor;
}


/**
 * If copy constructor is available, generate a call to it;
 * otherwise make a copy of the expression's result and push
 * it to the debugged thread's stack.
 * The resulting expression is a pointer pointing to the stack.
 */
static RefPtr<Expr>
pass_by_value(Context& ctxt, Expr& expr, ClassType& klass)
{
    RefPtr<Expr> ctorExpr = make_copy_ctor_expr(expr);

    if (!ctorExpr)
    {
        //
        // todo: mimic compiler-generated copy ctor,
        //  i.e. do a member-wise copy; for now, just
        //  do a bitwise copy
        //
        if (RefPtr<Variant> v = expr.value())
        {
            const addr_t sp = ctxt.push_arg(*v);

            if (Runnable* task = interface_cast<Runnable*>(ctxt.thread()))
            {
                task->set_stack_pointer(sp);
            }
            RefPtr<DebugSymbol> ptr = make_pointer(ctxt, klass, sp);
            assert(interface_cast<PointerType*>(ptr->type()));

            ctorExpr = new Constant(expr.interp().get(), *ptr);
        }
    }
    return ctorExpr;
}


/**
 * Is the function we're about to call a constructor?
 * If yes, synthesize and return the symbol that corresponds
 * to the THIS parameter of the ctor.
 */
static RefPtr<DebugSymbol> check_ctor_invocation
(
    Interp&                 interp,
    const FunType&          proto,
    const SharedString*     fname,
    ExprList*&              args
)
{
    RefPtr<DebugSymbol> result;

    const size_t argCount = args ? args->size() : 0;

    if (fname && (proto.param_count() == argCount + 1))
    {
        RefPtr<DataType> type = proto.param_type(0);
        type = remove_qualifiers(type); // strip const-volatile
        if (PointerType* ptr = interface_cast<PointerType*>(type.get()))
        {
            if (ClassType* klass =
                interface_cast<ClassType*>(ptr->pointed_type()))
            {
                string ctor = klass->name()->c_str();
                ctor += "::";
                ctor += klass->unqualified_name()->c_str();

                const size_t n = ctor.size();
                if (strncmp(fname->c_str(), ctor.c_str(), n) == 0
                    && fname->c_str()[n] == '(')
                {
                    if (!args)
                    {
                        // note: the static variable is okay here,
                        // because the expression interpreter cannot
                        // be multithreaded; it can only run on the
                        // thread that has issued a PT_ATTACH command
                        // (because it needs to manipulate the debuggee).
                        static ExprList argList;
                        argList.clear();
                        args = &argList;
                    }
                    result = make_this_param(interp, *klass, *args);
                }
            }
        }
    }
    return result;
}


/**
 * @return true if both types are numeric
 * @note it is not folded into equivalent_arg because conversion
 * (to adjust the type width) may still be needed
 */
bool numeric_types(const DataType* t1, const DataType* t2)
{
    if ((interface_cast<const IntType*>(t1)
        || interface_cast<const FloatType*>(t1))
        &&
        (interface_cast<const IntType*>(t2)
        || interface_cast<const FloatType*>(t2))
       )
    {
        return true;
    }
    return false;
}


/**
 * Helper for evaluating function calls.
 * Verify that the argument list matches the function prototype
 * and do type conversions if neeeded. Return a CallSetup object
 * if successful, throw otherwise.
 *
 * @todo revisit this function
 */
RefPtr<CallSetup> get_call_setup
(
    Context&            ctxt,
    Expr&               expr,
    const FunType&      proto,
    DebugSymbol*        fun,    // may be NULL
    const SharedString* fname,  // may be NULL
    ExprList*           args
)
{
    DEBUG_OUT << proto.name() << endl;

    RefPtr<CallSetup> setup(expr.call_setup());
    if (setup.get())
    {
        return setup;
    }
    ExprList params;
    size_t conversions = 0;

    // number of parameters required by function prototype
    const size_t paramCount = proto.param_count();

    Interp& interp = *CHKPTR(expr.interp());
    RefPtr<DebugSymbol> thisPtr =
        check_ctor_invocation(interp, proto, fname, args);

    if (!args || args->empty())
    {
        if (paramCount == 0)
        {
            setup = CallSetup::create(&expr, proto);
        }
    }
    else if (args->size() == paramCount
         || (args->size() > paramCount && var_args(proto, fname)))
    {
        // number of args matches prototype, check types
        const size_t count = args->size();
        for (size_t i = 0; i != count; ++i)
        {
            const RefPtr<Expr>& arg = (*args)[i];
            eval_arg(ctxt, arg);
            assert(arg->type()); // by eval_arg contract

            if (i >= paramCount)
            {
                assert(var_args(proto, fname));
                params.push_back(arg);
                continue;
            }
            RefPtr<DataType> paramType = proto.param_type(i);

            if (equivalent_arg(paramType, arg->type().get(), i, fun, &conversions))
            {
                if (ClassType* klass = interface_cast<ClassType*>(arg->type().get()))
                {
                    RefPtr<Expr> ctorExpr = pass_by_value(ctxt, *arg, *klass);

                    if (!ctorExpr)
                    {
                        throw runtime_error("copy constructor not found");
                    }
                }
                params.push_back(arg);
            }
            else if (ctxt.conversion_count() &&
                !numeric_types(paramType.get(), arg->type().get()))
            {
                // only one user-conversion is allowed per param
                // DEBUG_OUT << proto.name() << ": discarded\n";
                assert(!setup);
                return setup;
            }
            else
            {
                DEBUG_OUT << "conversion from " << arg->type()->name()
                          << " to " << paramType->name() << endl;

                RefPtr<Expr> type(new TypeName(NULL, paramType));
                RefPtr<Expr> cast;
                // if the first argument of a virtual function
                // call, use a static cast, so that base-to-derived
                // conversions are allowed for the THIS pointer
                if (i == 0)
                {
                    Method* method = interface_cast<Method*>(fun);

                    DEBUG_OUT << "*** method=" << method << "***" << endl;

                    if (method && method->is_virtual())
                    {
                        cast = new StaticCast(type, arg);
                    }
                }
                // the type does not match what the function
                // prototype requires, try implicit conversion
                if (!cast)
                {
                    cast = new ImplicitCast(type, arg);
                    ++conversions;
                }
                params.push_back(cast);
            }
        }
        setup = CallSetup::create(&expr, params, proto, conversions);
    }
    else
    {
        DEBUG_OUT << paramCount << " expected, "
                  << args->size() << " supplied" << endl;
    }

    if (thisPtr && setup)
    {
        // force it the return the address of the object we have
        // allocated on the debuggee's stack; force returned type
        // to be of pointer-to-class

        setup->set_ret_addr(thisPtr->addr());
        setup->set_ret_type(thisPtr->type());
    }
    return setup;
}


// keep all possible candidates sorted by the number
//  of required parameter type conversions
typedef multimap<size_t, RefPtr<CallSetup> >  CandidateMap;


static RefPtr<CallSetup>
pick_candidate(Context& context, const CandidateMap& candidates)
{
    RefPtr<CallSetup> result;

    if (!candidates.empty())
    {
        CandidateMap::const_iterator i = candidates.begin();

        result = i->second;

    #if 0
        // if several candidates have the same number
        // of required conversions, then try to pick
        // the one defined in the main program module
        // (if there is one)

        if (RefPtr<Thread> thread = context.thread())
        {
            RefPtr<SymbolMap> symbols = thread->symbols();

            const size_t score = i->first;

            for (; (i != candidates.end()) && (i->first == score); ++i)
            {
                // address of function to call
                addr_t addr = i->second->addr();

                if (RefPtr<Symbol> sym = symbols->lookup_symbol(addr))
                {
                    if (RefPtr<SymbolTable> table = sym->table())
                    {
                        DEBUG_OUT << table->filename() << endl;

                        if (table->filename()->is_equal(thread->name()))
                        {
                            result = i->second;
                            break;
                        }
                    }
                }
            }
        }
    #endif
    }

    return result;
}


/**
 * Helper for evaluating function calls.
 */
RefPtr<CallSetup> get_call_setup
(
    Context&        ctxt,
    Expr&           expr,
    DebugSymbol*    sym,
    ExprList*       args
)
{
    if (!sym)
    {
        throw logic_error("null function symbol");
    }

    FunType* funType = interface_cast<FunType*>(sym->type());
    if (!funType)
    {
        // check for pointer-to-function, so that we can
        // call fp(), not just (*fp)()
        PointerType* ptrType = interface_cast<PointerType*>(sym->type());
        if (ptrType)
        {
            funType = interface_cast<FunType*>(ptrType->pointed_type());
            if (!sym->enum_children())
            {
                sym->read(&ctxt);
            }
            sym = sym->nth_child(0);
        }
    }
    if (!funType)
    {
        assert(sym->name());
        const char* name =CHKPTR(sym->name())->c_str();
        throw EvalError(name + string(" is not a function"));
    }
    RefPtr<CallSetup> setup;

    if (const size_t n = sym->enum_children())
    {
        // overloaded function?
        CandidateMap candidates;

        for (size_t i = 0; i != n; ++i)
        {
            DebugSymbol* fun = CHKPTR(sym->nth_child(i));
            funType = interface_cast<FunType*>(fun->type());
            CHKPTR(funType);
            setup = get_call_setup(ctxt, expr, fun, args);
            DEBUG_OUT << funType->name() << " setup=" << setup.get() << endl;
            if (setup)
            {
                const size_t count = setup->conversion_count();
                DEBUG_OUT << fun->value() << " conversions: " << count << endl;

                candidates.insert(make_pair(count, setup));
            }
        }
        // pick the function that matches the parameters and
        // requires the least conversions
     #if 0
        if (!candidates.empty())
        {
            setup = candidates.begin()->second;
        }
     #else
        setup = pick_candidate(ctxt, candidates);
     #endif
        assert(!setup || setup->addr());
    }
    else
    {
        assert(sym->addr());
        setup = get_call_setup( ctxt,
                                expr,
                                *funType,
                                sym,
                                sym->value(),
                                args);
        if (setup.get())
        {
            setup->set_addr(sym->addr());
            setup->set_reader(sym->reader());
        }
    }
    return setup;
}


static void params_to_string(FunType& funType, string& s)
{
    s += "(";
    const size_t n = funType.param_count();
    for (size_t i = 0; i != n; ++i)
    {
        if (i)
        {
            s += ", ";
        }
        SharedString* name = CHKPTR(funType.param_type(i))->name();
        s += CHKPTR(name)->c_str();
    }
    if (funType.has_variable_args())
    {
        if (n)
        {
            s += ", ";
        }
        s += "...";
    }
    s += ")";
}


/**
 * Construct error message for the case when a function call
 * expression  does not match any function's prototype; possible
 * candidates are included in the error message.
 */
void
no_matching_func(Context& ctxt, DebugSymbol& fun, ExprList* args)
{
    string err = "No matching function for call to ";
    assert(fun.name());

    err += fun.name()->c_str();
    err += "(";

    if (args)
    {
        ExprList::const_iterator i = args->begin();
        for (; i != args->end(); ++i)
        {
            if (i != args->begin())
            {
                err += ", ";
            }
            (*i)->eval(ctxt);
            SharedString* name = (*i)->type()->name();
            err += CHKPTR(name)->c_str();
        }
    }
    err += ")\nCandidates are:\n";

    if (const size_t numOverloads = fun.enum_children())
    {
        for (size_t i = 0; i != numOverloads; ++i)
        {
            if (i)
            {
                err += "\n";
            }
            DebugSymbol* f = CHKPTR(fun.nth_child(i));
            if (f->value())
            {
                err += f->value()->c_str();
            }
        }
    }
    else
    {
        string fname = CHKPTR(fun.value())->c_str();
        err += fname;

        if (FunType* funType = interface_cast<FunType*>(fun.type()))
        {
            params_to_string(*funType, err);
        }
    }
    throw EvalError(err);
}



/**
 * Construct and add to argument list an expression that
 * corresponds to the hidden THIS pointer; the type of the
 * argument is given by the ClassType.
 */
RefPtr<DebugSymbol>
make_this_param(Interp& interp, ClassType& klass, ExprList& args)
{
    Context& ctxt = interp.context();

    // allocate an object on the stack
    RefPtr<VariantImpl> object = new VariantImpl();
    object->resize(klass.size());
    const addr_t addr = ctxt.push_arg(*object);

    // make a constant expression of the object's address
    RefPtr<DebugSymbol> sym = make_pointer(ctxt, klass, addr);
    args.insert(args.begin(), new Constant(&interp, *sym));
    return sym;
}


RefPtr<CallSetup> get_ctor_setup
(
    Context&        ctxt,
    Expr&           expr, // expression invoking ctor
    ClassType&      klass,// C++ class in the debuggee
    addr_t          addr, // address where to construct
    DataType*       thisType,
    ExprList&       args  // desired ctor arg list
)
{
    RefPtr<CallSetup> setup; // result

    const size_t count = klass.method_count();
    for (size_t i = 0; i != count; ++i)
    {
        const Method* method = klass.method(i);
        DEBUG_OUT << "\'" << method->name() << "\'" << endl;

        if (!method->name()->is_equal2(klass.unqualified_name())
         && !method->name()->is_equal("__base_ctor")
         && !method->name()->is_equal("__comp_ctor"))
        {
            continue; // not a ctor
        }
        RefPtr<FunType> funType = method->type();
        if (!funType)
        {
            continue;
        }
        const size_t paramCount = funType->param_count();

        DEBUG_OUT << funType->name() << endl;
        DEBUG_OUT << "param count=" << paramCount << endl;

        // note: it is possible for the ctor to have var args but
        // that would be bad C++ style, and thus not worth bothering with...
        if (paramCount < args.size())
        {
            continue;
        }
        // try finding a matching function signature
        setup = get_call_setup(ctxt, expr, *funType, 0, 0, &args);
        if (!setup)
        {
            DEBUG_OUT << "try " << i << ": no match\n";
        }
        else
        {
            setup->set_addr(method->start_addr());
            if (method->linkage_name() != method->name())
            {
                setup->set_fname(method->linkage_name());
            }
            else
            {
                string name(klass.name()->c_str());
                name += "::";
                name += method->name()->c_str();
                setup->set_fname(shared_string(name).get());
            }
            if (!ctxt.get_func(*setup))
            {
                continue;
            }
            // force it the return the address of the object we have
            // allocated on the debuggee's stack; force returned type
            // to be of pointer-to-class
            setup->set_ret_addr(addr);
            setup->set_ret_type(thisType);
            break;
        }
    }
    return setup;
}



#ifdef DEBUG
void dump_args(Context& ctxt, const ExprList& args)
{
    if (Interp::debug_enabled())
    {
        clog << "--- arguments: ----\n";
        for (ExprList::const_iterator i = args.begin();
            i != args.end();
            ++i)
        {
            variant_print(clog, *(*i)->eval(ctxt));
            clog << " (" << (*i)->_name() << " type=";
            clog << (*i)->type()->name()  << ")\n";
        }
    }
}
#endif
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
