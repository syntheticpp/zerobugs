#ifndef ENVIRONMENT_H__4A7E2F28_84CB_4F1B_BB81_DA37B0B2A55C
#define ENVIRONMENT_H__4A7E2F28_84CB_4F1B_BB81_DA37B0B2A55C
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: environment.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "generic/export.h"


/**
 * Interface for getting / modifying the environment.
 */
struct ZDK_LOCAL Environment
{
    virtual const char* const* get(bool reset = false) = 0;
    virtual void set(const char* const*) = 0;
};

#endif // ENVIRONMENT_H__4A7E2F28_84CB_4F1B_BB81_DA37B0B2A55C
