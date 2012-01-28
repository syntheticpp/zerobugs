#ifndef SYMBOL_MAP_H__316D8291_4A2D_4147_8004_E1F67E3193EA
#define SYMBOL_MAP_H__316D8291_4A2D_4147_8004_E1F67E3193EA
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

/* forward decls */
namespace ELF
{
    class CoreFile;
}
class DynLibList;
class Process;
class SymbolTableEvents;


/**
 * Read the symbol map for a core file.
 */
RefPtr<SymbolMap>
read_symbols_from_core_dump(Process&, const ELF::CoreFile&, SymbolTableEvents&);


/**
 * Read the symbol map of a running program.
 */
RefPtr<SymbolMap>
read_symbols_from_process(Process&, SymbolTableEvents&);


#endif // SYMBOL_MAP_H__316D8291_4A2D_4147_8004_E1F67E3193EA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
