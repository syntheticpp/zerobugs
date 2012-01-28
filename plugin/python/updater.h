#ifndef UPDATER_H__083DD28F_A5BB_48AA_87C1_362D6EA83F11
#define UPDATER_H__083DD28F_A5BB_48AA_87C1_362D6EA83F11
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include "zdk/update.h"
#include "zdk/zobject_impl.h"
#include "interp.h"


/**
 * Implements Update support by invoking a python script.
 */
CLASS Updater : public ZObjectImpl<Updateable>
{
    RefPtr<Python> python_;

public:
    explicit Updater(const RefPtr<Python>&);

    size_t check_for_updates(EnumCallback<Update*>*);
};

#endif // UPDATER_H__083DD28F_A5BB_48AA_87C1_362D6EA83F11
