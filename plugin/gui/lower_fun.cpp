//
// $Id: lower_fun.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/debug_sym.h"
#include "zdk/module.h"
#include "zdk/shared_string_impl.h"
#include "zdk/zobject_scope.h"
#include "dharma/symbol_util.h"
#include "lower_fun.h"

using namespace std;


addr_t lower_fun(Debugger& debugger, const Symbol& sym)
{
    ZObjectScope scope;
    RefPtr<Process> proc;

    SymbolTable* table = sym.table(&scope);
    if (table)
    {
        proc = table->process(&scope);
    }
    if (proc)
    {
        if (proc->vdso_symbol_tables() == table)
        {
            return 0;
        }
    }
    addr_t result = 0;

    const addr_t addr = sym.addr();
    if (RefPtr<TranslationUnit> unit = debugger.lookup_unit_by_addr(proc.get(), addr))
    {
        result = unit->lower_addr();
    }
    return result;
}


/**
 * Find the function with the lower address defined
 * in the translation unit whose file name is `srcPath'
 * return a null pointer if no function found
 */
RefPtr<Symbol> lower_fun(Debugger& dbg, const char* srcPath)
{
/*** BEGIN callback ***/
    class ModuleCallback
        : public  EnumCallback<Module*>
        , private EnumCallback<DebuggerPlugin*>
        , private EnumCallback<addr_t>
    {
    public:
        virtual ~ModuleCallback() {}

        ModuleCallback(Debugger& debugger, const char* path)
            : debugger_(debugger)
            , path_(shared_string(path))
            , addr_(0)
            , cancel_(false)
        {
            if (Thread* thread = debugger.get_thread(DEFAULT_THREAD))
            {
                process_ = thread->process();
            }
        }

        void notify(Module* mod)
        {
            if (!cancel_)
            {
                assert(mod);
                mod_ = mod;
                debugger_.enum_plugins(this);
            }
        }

        void notify(addr_t addr)
        {
            assert(mod_.get());
            if (addr && ((addr_ == 0) || (addr < addr_)))
            {
                assert_gte(addr, mod_->adjustment());
                addr_ = addr;
                best_ = mod_;
            }
        }

        // callback for enum_plugins()
        void notify(DebuggerPlugin* p)
        {
            if (DebugInfoReader* r = interface_cast<DebugInfoReader*>(p))
            {
                if (process_)
                {
                    r->line_to_addr(process_.get(),
                                    mod_->name(),
                                    mod_->adjustment(),
                                    path_.get(), 0, this,
                                    &cancel_);
                }
            }
        }

        RefPtr<Symbol> symbol() const
        {
            if (addr_ && symbol_.is_null() && !best_.is_null())
            {
                SymbolTable* tbl = best_->symbol_table_list();
                for (; tbl && symbol_.is_null(); tbl = tbl->next())
                {
                    if (!tbl->is_dynamic())
                    {
                        symbol_ = tbl->lookup_symbol(addr_);
                    }
                }
            }

            return symbol_;
        }

    private:
        Debugger&               debugger_;
        RefPtr<Process>         process_;
        RefPtr<SharedString>    path_;
        RefPtr<Module>          mod_;
        RefPtr<Module>          best_;
        addr_t                  addr_;
        mutable RefPtr<Symbol>  symbol_;
        bool                    cancel_;
    };
/*** END callback ***/

    ModuleCallback moduleCB(dbg, srcPath);
    dbg.enum_modules(&moduleCB);

    return moduleCB.symbol();
}
// Copyright (c) 2004, 2006 Cristian L. Vlasceanu

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
