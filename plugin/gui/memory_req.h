#ifndef MEMORY_REQ_H__2E4F52FA_40DE_4AFD_91DC_75A97A66C734
#define MEMORY_REQ_H__2E4F52FA_40DE_4AFD_91DC_75A97A66C734
//
// $Id: memory_req.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <vector>
#include "zdk/ref_counted_impl.h"
#include "view_types.h"

class SharedString;
class Symbol;



/**
 * Helper class for reading memory asynchronously --
 * used when disassembling the debugged program
 */
struct MemoryRequest : public RefCountedImpl<>
{
    MemoryRequest(RefPtr<Symbol>,
                 size_t,
                 ViewType,
                 bool,
                 RefPtr<SharedString>);

    virtual ~MemoryRequest() throw();

    RefPtr<Symbol> begin;   // where to start reading from

    size_t requestedSize;   // how much to read

    ViewType viewType;      // the current view type

    bool hilite;            // hilight the symbol?

    std::vector<uint8_t> bytes;

    RefPtr<SharedString> file;
};

#endif // MEMORY_REQ_H__2E4F52FA_40DE_4AFD_91DC_75A97A66C734
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
