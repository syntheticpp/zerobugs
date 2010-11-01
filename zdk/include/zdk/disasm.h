#ifndef DISASM_H__83D587B3_47AE_4D79_885E_8114D8B372BD
#define DISASM_H__83D587B3_47AE_4D79_885E_8114D8B372BD
//
// $Id: disasm.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/enum.h"
#include "zdk/platform.h"
#include "zdk/plugin.h"
#include <string>
#include <vector>

using Platform::addr_t;

struct SharedString;
struct Symbol;


DECLARE_ZDK_INTERFACE_(Disassembler, Plugin)
{
    DECLARE_UUID("f313e09b-3358-4e81-a3b1-7a4eaf5399bb")

    typedef EnumCallback2<
        addr_t,
        bool,
        const Symbol*> SymbolCallback;

    typedef EnumCallback2<
        addr_t,
        size_t,
        const std::vector<std::string>*> SourceCallback;

    struct OutputCallback : public EnumCallback3<addr_t,
                                                 const char*,
                                                 size_t,
                                                 bool>
    {
        virtual bool tabstops(size_t*, size_t*) const = 0;
    };


    enum Syntax
    {
        ASM_FIXED, // hardcoded, cannot be changed
        ASM_ATT,
        ASM_INTEL
    };

    /**
     * @return true if disassemble() does not need
     * a memory buffer to be provided but uses its
     * own internal reading.
     */
    virtual bool uses_own_buffer() const = 0;

    virtual size_t disassemble(
        addr_t          startAddr,
        off_t           adjustment,
        const uint8_t*  memBuffer,
        size_t          length,
        SharedString*   filename = 0,
        OutputCallback* outputLineCB = 0,
        SymbolCallback* lookupSymbolCB = 0,
        SourceCallback* lookupSourceCB = 0) = 0;

    virtual Syntax syntax() const = 0;

    virtual void set_syntax(Syntax) = 0;
};

#endif // DISASM_H__83D587B3_47AE_4D79_885E_8114D8B372BD
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
