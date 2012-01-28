#ifndef CONTEXT_IMPL_H__D6F37D1C_96B7_4B71_8A21_D03D83069253
#define CONTEXT_IMPL_H__D6F37D1C_96B7_4B71_8A21_D03D83069253
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
//
#include "zdk/ref_counted_impl.h"
#include "context.h"
#include <memory>
#include <vector>

class FrameSetup;
class Symbol;
class Thread;


CLASS ContextImpl : public RefCountedImpl<Context>
{
protected:
    typedef std::auto_ptr<FrameSetup> FrameSetupPtr;

    explicit ContextImpl(Thread&, addr_t = 0);

public:
    virtual ~ContextImpl() throw();

    static RefPtr<Context> create(Thread&, addr_t = 0);

protected:
    //
    // Context interface
    //
    RefPtr<DataType> lookup_type(const char* name);

    RefPtr<DebugSymbol> lookup_debug_symbol(const char*, LookupOpt);

    void commit_value(DebugSymbol&, Variant&);

    TypeSystem& type_system() const;

    /**
     * Fabricate a temporary pointer symbol.
     */
    RefPtr<DebugSymbol> new_temp_ptr(PointerType&, addr_t);

    RefPtr<DebugSymbol> new_const(DataType&, const std::string&);

    addr_t push_arg(const Variant& v, addr_t sp = 0);

    void reset_frame_setup();
    void call_function(CallSetup&);

    /**
     * This function assumes that a C++ class instance lives at the
     * given address "addr", and reads the vtable pointer, vptr, from
     * that address. It then reads the address at "index" from the vtable
     * (i.e. the index-th value in the vtable), and uses it to lookup
     * a function, by address.
     */
    RefPtr<DebugSymbol> get_virt_func(addr_t, off_t);

    Thread* thread() const { return thread_.get(); }

    // DebugSymbolEvents interface
    bool notify(DebugSymbol*);

    bool is_expanding(DebugSymbol*) const;

    int numeric_base(const DebugSymbol*) const;

    void symbol_change(DebugSymbol*, DebugSymbol*) { };

private:
    RefPtr<Symbol> get_func(const CallSetup&) const;

    typedef std::vector<RefPtr<Variant> > VariantList;
    /**
     * Evaluate the arguments of a function call, apply
     * implicit casts where needed.
     */
    VariantList eval_args(Context&, CallSetup&);
    addr_t eval_args(Context&, CallSetup&, VariantList&);

    /**
     * Pass as many parameters as possible into registers --
     * depending on the architecture. The parameters that are
     * copied to cpu registers are removed from the input vector.
     * @return true if any registers were used
     */
    bool pass_by_reg(std::vector<RefPtr<Variant> >&,
                     addr_t& stackPtr);

    addr_t check_ret_by_value(CallSetup&, Symbol&);

    addr_t get_stack_top() const;

    RefPtr<Context> spawn() const;

    /**
     * Helper called from notify() in the event of
     * several symbol matches
     */
    void assign(const RefPtr<DebugSymbol>&);

    void ensure_frame_setup();

private:
    RefPtr<Thread>      thread_;        // thread of context
    RefPtr<Symbol>      function_;      // for scope resolution
    RefPtr<DebugSymbol> debugSym_;      // memorizes last lookup
    bool                collect_;
    FrameSetupPtr       frameSetup_;
};

#endif // CONTEXT_IMPL_H__D6F37D1C_96B7_4B71_8A21_D03D83069253
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
