#ifndef RTTI_DISABLER_H__01463DD9_82F1_455E_9017_1CE2F58E0F34
#define RTTI_DISABLER_H__01463DD9_82F1_455E_9017_1CE2F58E0F34
//
// $Id: rtti_disabler.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/type_system.h"


/**
 * Temporarily turns off auto-RTTI in its scope.
 */
CLASS RTTI_Disabler : boost::noncopyable
{
    bool rtti_;
    TypeSystem& typeSys_;
public:
    explicit RTTI_Disabler(TypeSystem& typeSys)
        : rtti_(typeSys.use_auto_rtti())
        , typeSys_(typeSys)
    {
        typeSys.set_auto_rtti(false);
    }
    ~RTTI_Disabler() { typeSys_.set_auto_rtti(rtti_); }
};

#endif // RTTI_DISABLER_H__01463DD9_82F1_455E_9017_1CE2F58E0F34
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
