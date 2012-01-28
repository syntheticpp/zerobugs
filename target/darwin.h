#ifndef DARWIN_H__AAD710A2_FE29_4B1B_BCFB_8118E39558B3
#define DARWIN_H__AAD710A2_FE29_4B1B_BCFB_8118E39558B3
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
#include "unix.h"

/**
 * Common base for Darwin/OSX debug targets
 */
class DarwinTarget : public UnixTarget
{
protected:
    explicit DarwinTarget(debugger_type& debugger)
        : UnixTarget(debugger) { }

    ~DarwinTarget() throw();

public:
    virtual long syscall_num(const Thread&) const
    { return -1; }

    /// @note linux-specific, just return NULL here
    virtual RefPtr<SymbolTable> vdso_symbol_tables() const
    { return RefPtr<SymbolTable>(); }


private:
};


#endif // DARWIN_H__AAD710A2_FE29_4B1B_BCFB_8118E39558B3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
