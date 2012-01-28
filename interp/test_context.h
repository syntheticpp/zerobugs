#ifndef TEST_CONTEXT_H__990C4E62_784E_40DD_ABE0_B11476549BDF
#define TEST_CONTEXT_H__990C4E62_784E_40DD_ABE0_B11476549BDF
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
#include <string>
#include "zdk/zobject_impl.h"
#include "context.h"


class TestContext : public ZObjectImpl<Context>
{
public:
    TestContext();

    ~TestContext() throw();

    bool add_debug_symbol(RefPtr<DebugSymbol>);

    ///// Context interface /////
    RefPtr<DataType> lookup_type(const char* name);

    RefPtr<DebugSymbol> lookup_debug_symbol(const char*, LookupOpt);

    void commit_value(DebugSymbol&, Variant&);

    TypeSystem& type_system() const;

    RefPtr<DebugSymbol> new_temp_ptr(PointerType&, addr_t);

    RefPtr<DebugSymbol> new_const(DataType& type, const std::string& value);

    void call_function(CallSetup&);

    RefPtr<Symbol> get_func(const CallSetup& setup) const;

    addr_t push_arg(const Variant& v, addr_t sp);

    ///// DebugSymbolCallback interface /////

    bool notify(DebugSymbol*);

    bool is_expanding(DebugSymbol*) const;

    int numeric_base(const DebugSymbol*) const { return 16; }

    //void type_changed(DebugSymbol*, const SharedString*) {};
    void symbol_change(DebugSymbol* newSym, DebugSymbol* old)
    { }

    RefPtr<Context> spawn() const { return new TestContext; }

    void reset_frame_setup() { }

private:
    typedef std::map<std::string, RefPtr<DebugSymbol> > DebugSymbolMap;

    DebugSymbolMap debugSymbolMap_;

    mutable std::auto_ptr<TypeSystem> types_;
};
#endif // TEST_CONTEXT_H__990C4E62_784E_40DD_ABE0_B11476549BDF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
