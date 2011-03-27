#ifndef IS_REFERENCE_H__AEEEDF41_66DD_4578_AD01_6351B8A57FF6
#define IS_REFERENCE_H__AEEEDF41_66DD_4578_AD01_6351B8A57FF6
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: is_reference.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

inline bool is_ref(const RefPtr<DebugSymbol>& sym)
{
    bool isRef = false;
    if (PointerType* pt = interface_cast<PointerType*>(sym->type()))
    {
        isRef = pt->is_reference();
    }
    return isRef;
}
#endif // IS_REFERENCE_H__AEEEDF41_66DD_4578_AD01_6351B8A57FF6
