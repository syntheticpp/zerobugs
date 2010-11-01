#ifndef FRAME_STATE_H__F5B61287_9455_4DB6_91E2_CC2E17B6A870
#define FRAME_STATE_H__F5B61287_9455_4DB6_91E2_CC2E17B6A870
//
// $Id: frame_state.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <map>
#include <set>
#include "symkey.h"
#include "zdk/debug_symbol_list.h"
#include "zdk/zobject_impl.h"



struct FrameState : public ZObjectImpl<>
{
    DECLARE_UUID("51de5f25-e19b-42e8-89ce-d082e2f0169f")

    BEGIN_INTERFACE_MAP(FrameState)
        INTERFACE_ENTRY(FrameState)
    END_INTERFACE_MAP()

    virtual ~FrameState() throw() {};

    std::map<SymKey, RefPtr<SharedString> > values_;
    std::set<SymKey> expand_;
    DebugSymbolList symbols_;
};
#endif // FRAME_STATE_H__F5B61287_9455_4DB6_91E2_CC2E17B6A870
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
