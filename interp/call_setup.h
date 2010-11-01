#ifndef CALL_SETUP_H__185FAB1F_820D_4C1E_A8B4_A24C2E12F10D
#define CALL_SETUP_H__185FAB1F_820D_4C1E_A8B4_A24C2E12F10D
//
// $Id: call_setup.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "expr.h"

/**
 * Encapsulates information about a function inside
 * the debugged program that is about to be called
 * from the expression interpreter. The context
 * sets up the stack for the call based on this info
 * (hence the name CallSetup).
 */
CLASS CallSetup : public RefCountedImpl<>
{
public:
    virtual ~CallSetup() throw();

protected:
    CallSetup
    (
        Expr*,
        const ExprList&,
        const FunType&,
        size_t = 0
    );

    CallSetup(Expr*, const FunType&);

public:
    static RefPtr<CallSetup>
    create(Expr* expr, const ExprList& list, const FunType& fun, size_t size)
    {
        return new CallSetup(expr, list, fun, size);
    }
    static RefPtr<CallSetup>
    create(Expr* expr, const FunType& fun)
    {
        return new CallSetup(expr, fun);
    }

    /**
     * The function to be called from the interpreter
     * can be given either by its address, or by its
     * name.
     *
     * Set the address of the function to call.
     */
    void set_addr(addr_t addr)
    {
        assert(!addr_ || addr_ == addr);

        addr_ = addr;
    }

    /**
     * @return the address of the function to call
     */
    addr_t addr() const { return addr_; }

    /**
     * The function to be called from the interpreter
     * can be given either by its address, or by its
     * name.
     *
     * Set the name of the function to call.
     */
    void set_fname(SharedString* fname)
    {
        assert(!fname_);
        fname_ = fname;
    }

    /**
     * @return the name of the function to call
     */
    SharedString* fname() const { return fname_.get(); }

    /**
     * Force the return address to the given value,
     * rather than using whatever the debuggee function
     * call returns -- useful for calling ctors and funcs
     * that return objects by value.
     */
    void set_ret_addr(addr_t addr) { retAddr_ = addr; }

    addr_t ret_addr() const { return retAddr_; }

    RefPtr<DataType> ret_type() const { return retType_; }

    /**
     * Force the return type -- useful when calling
     * constructors in static_cast<> expressions.
     */
    void set_ret_type(const RefPtr<DataType>& type)
    {
        assert(type);
        retType_ = type;
    }

    ExprList& args() { return args_; }
    const ExprList& args() const { return args_; }

    RefPtr<Expr> expr() { return expr_.ref_ptr(); }

    void set_reader(DebugInfoReader* reader)
    {
        assert(!reader_ || reader_ == reader); // set once
        reader_ = reader;
    }

    DebugInfoReader* reader() const { return reader_; }

    bool is_return_type_strict() const
    { return proto_.is_return_type_strict(); }

    bool has_variable_args() const;

    size_t conversion_count() const { return conversions_; }

private:
    addr_t                  addr_;  // address of function to call
    addr_t                  retAddr_;
    ExprList                args_;  // function arguments
    WeakPtr<Expr>           expr_;
    RefPtr<SharedString>    fname_;
    RefPtr<DataType>        retType_;
    DebugInfoReader*        reader_; // may be NULL
    const FunType&          proto_;  // owned by TypeSystem
    size_t                  conversions_;
};


class FunType; // for full definition see: zdk/types.h

RefPtr<CallSetup> get_call_setup(
    Context&            ctxt,
    Expr&               expr,
    const FunType&      proto,
    DebugSymbol*        fun,
    const SharedString* name,
    ExprList*           args);

/**
 * Helper for evaluating function calls.
 */
RefPtr<CallSetup> get_call_setup(
    Context&        ctxt,
    Expr&           expr,
    DebugSymbol*    sym,
    ExprList*       args);


RefPtr<CallSetup> get_ctor_setup(
    Context&        ctxt,
    Expr&           expr, // expression invoking ctor
    ClassType&      klass,// C++ class in the debuggee
    addr_t          addr, // address where to construct
    DataType*       thisType,
    ExprList&       args);// ctor arg list, must include THIS


/**
 * Construct an error message for the case when a function
 * call expression does not match any function's prototype;
 * possible candidates are included in the error message.
 */
void no_matching_func(Context&, DebugSymbol&, ExprList*);


/**
 * @return true if a variabile of type T2 can be passed
 * as an argument to a function that expects a parameter
 * of type T1, according to C++ language rules.
 * If the conversionCount parameter is non-NULL, the value it points
 * onto will contain the number of conversions required.
 */
bool
equivalent_arg( RefPtr<DataType> t1,
                const DataType* t2,
                int argIndex = -1,
                DebugSymbol* function = 0,
                size_t* conversionCount = 0);


/**
 * @return true if both types are non-complex numeric
 * (i.e. integers or real)
 */
bool numeric_types(const DataType*, const DataType*);


/**
 * Construct and add to argument list an expression that
 * corresponds to the hidden THIS pointer; the type of the
 * argument is given by the ClassType.
 */
RefPtr<DebugSymbol>
make_this_param(Interp&, ClassType&, ExprList&);


#ifdef DEBUG
 void dump_args(Context& ctxt, const ExprList& args);
#else
 #define dump_args(x,y)
#endif

#endif // CALL_SETUP_H__185FAB1F_820D_4C1E_A8B4_A24C2E12F10D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
