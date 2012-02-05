#include "zdk/zero.h"
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
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/interface_cast.h"

namespace
{
    /**
     * Helper for enumerating plugins; get_return_symbol is
     * called for plugins that implement the DebugInfoReader
     * interface, until a non-null debug symbol is found.
     */
    class ZDK_LOCAL ReturnSymbolHelper : public EnumCallback<DebuggerPlugin*>
    {
    public:
        ReturnSymbolHelper(Thread* thread, RefPtr<Symbol> symbol)
            : thread_(thread), symbol_(symbol)
        {
        }
        virtual ~ReturnSymbolHelper() throw()
        {
        }
        void notify(DebuggerPlugin* p)
        {
            if (retSym_)
            {
                return;
            }
            if (DebugInfoReader* r = interface_cast<DebugInfoReader*>(p))
            {
                retSym_ = r->get_return_symbol(thread_.get(), symbol_.get());
            }
        }
        RefPtr<DebugSymbol> ret_symbol() const
        {
            return retSym_;
        }

    private:
        RefPtr<DebugSymbol> retSym_;
        RefPtr<Thread>      thread_;
        RefPtr<Symbol>      symbol_;
    };
} // namespace



RefPtr<DebugSymbol> ret_symbol(Thread* thread, RefPtr<Symbol> symbol)
{
    ReturnSymbolHelper callback(thread, symbol);
    CHKPTR(thread->debugger())->enum_plugins(&callback);

    return callback.ret_symbol();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
