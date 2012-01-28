#ifndef CONTEXT_H__11C568D0_E035_4E90_A987_9C28E4764789
#define CONTEXT_H__11C568D0_E035_4E90_A987_9C28E4764789
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

#include <map>
#include <memory>
#include <stack>
#include "generic/temporary.h"
#include "zdk/debug_sym.h"
#include "zdk/expr.h"
#include "zdk/ref_ptr.h"


class CallSetup;
class DebugSymbol;
class PointerType;
class Expr;
class Interp;
class TypeSystem;
class Variant;



struct ZDK_LOCAL MacroHelper
{
    typedef std::map<std::string, RefPtr<Expr> > ArgMap;

    virtual ~MacroHelper() { }
    virtual void map_arguments(const std::string&, ArgMap&) = 0;
};


enum LookupOpt
{
    LKUP_NONE       = 0x0000,
    LKUP_FUNCS      = 0x0001, // include functions in lookup
    LKUP_FUNCS_ONLY = 0x0010, // lookup funcs only
    LKUP_COLLECT    = 0x0100, // fold multiple matches under parent
                              // symbol
    LKUP_DEFAULT    = LKUP_COLLECT,
};

inline LookupOpt operator|(LookupOpt lhs, LookupOpt rhs)
{
    return static_cast<LookupOpt>(static_cast<int>(lhs) | rhs);
}

/**
 * The expression interpreter does not recognize declarations;
 * rather, it uses the symbol tables and the debug information
 * from the debugged program.
 * This class provides a point of contact between the
 * interpreter and the rest of the debugger. This way, the
 * interpreter does not have to know about details such as the
 * current thread of execution, its stack trace, the current
 * stack frame in scope and so on; all these details make the
 * context for evaluating a given expression.
 * Furthermore, a special implementation of this abstract class can
 * help unit-testing the interpreter in a stand-alone environment.
 */
CLASS Context : public RefCounted, public DebugSymbolEvents
{
protected:
    Context();

public:
    DECLARE_UUID("56ca3cac-d624-4bc8-82de-3f1b6b72324d")

BEGIN_INTERFACE_MAP(Context)
    INTERFACE_ENTRY(Context)
    INTERFACE_ENTRY(DebugSymbolEvents)
    INTERFACE_ENTRY_DELEGATE(events_)
END_INTERFACE_MAP()

    virtual ~Context();

    /**
     * Lookup a data type by name
     */
    virtual RefPtr<DataType> lookup_type(const char* name) = 0;

    /**
     * Lookup debug symbol by name
     * @todo make sure it works well with statics and namespaces
     *  @param name
     * @param lookupFuncs true if functions should be included
     * in the result
     * @param collectFuncs true if overloads should be folded
     * as children of the result symbol
     */
    virtual RefPtr<DebugSymbol> lookup_debug_symbol(
        const char* name,
        LookupOpt = LKUP_DEFAULT) = 0;

    /**
     * Modify the value associated with a debug symbol in the
     * debuggee's memory
     */
    virtual void commit_value(DebugSymbol&, Variant& value) = 0;

    /**
     * Fabricate a temporary pointer symbol
     */
    virtual RefPtr<DebugSymbol> new_temp_ptr(PointerType&, addr_t) = 0;

    /**
     * Fabricate a fake DebugSymbol to represent a constant
     */
    virtual RefPtr<DebugSymbol> new_const(DataType&, const std::string&) = 0;

    /**
     * Pushes an argument to a function call onto the debuggee's stack.
     * If the stack pointer argument is zero, the current thread's SP
     * is assumed. Returns the new SP
     * @param v the argument to be pushed
     * @param setup the current setup for calling a function
     * @param sp the optional starting stack pointer
     */
    virtual addr_t push_arg(const Variant& v,
                            addr_t sp = 0) = 0;

    virtual void reset_frame_setup() = 0;

    virtual void call_function(CallSetup&) = 0;

    virtual RefPtr<Symbol> get_func(const CallSetup&) const = 0;

    virtual TypeSystem& type_system() const = 0;

    virtual RefPtr<Context> spawn() const = 0;

    /**
     * Assuming that a C++ object lives at address "addr", return
     * its "index-th" virtual function.
     */
    virtual RefPtr<DebugSymbol> get_virt_func(addr_t addr, off_t index)
    { return 0; }

    /**
     * @return the current thread associated with this context
     */
    virtual Thread* thread() const { return 0; }

    /**
     * convenience wrapper for talking to the the TypeSystem
     */
    RefPtr<DataType> get_int_type(bool isSigned = true) const;

    void push(const RefPtr<Expr>& expr);

    void pop();

    RefPtr<Expr> top() const;

    /**
     * @return number of successive type conversions
     */
    size_t conversion_count() const { return conversions_; }

    std::auto_ptr<Temporary<size_t> > increment_conversion_count();

    void set_events(ExprEvents*);

    RefPtr<ExprEvents> events() const;

    void notify_function_call_event(addr_t, Symbol*);

    void notify_function_return_event(addr_t addr)
    {
        notify_function_call_event(addr, NULL);
    }

    void notify_error_event(const char*, std::ostream* = NULL);
    void notify_warning_event(const char*, std::ostream* = NULL);

    bool notify_completion_event(Variant*, bool*);

    /**
     * Lookup an expression in the macro map
     */
    RefPtr<Expr> lookup_macro(Interp*, const std::string&) const;

    void set_macro_helper(MacroHelper* helper);

    void map_arguments(const std::string& str);

    void clone_expr_events()
    {
        if (events_)
        {
            events_.reset(events_->clone());
        }
    }

private:
    typedef std::stack<RefPtr<Expr> > ExprStack;

    ExprStack           stack_;
    size_t              conversions_;
    RefPtr<ExprEvents>  events_;

 protected:
    MacroHelper::ArgMap macroMap_;
    MacroHelper*        macroHelper_;
};



#endif // CONTEXT_H__11C568D0_E035_4E90_A987_9C28E4764789
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
