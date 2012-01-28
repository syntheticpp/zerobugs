#ifndef UPDATE_H__161E86D5_4E46_4C23_8C4E_D7F211C7FD39
#define UPDATE_H__161E86D5_4E46_4C23_8C4E_D7F211C7FD39
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
#include "zdk/enum.h"
#include "zdk/zobject.h"


DECLARE_ZDK_INTERFACE_(Update, ZObject)
{
    DECLARE_UUID("1200880c-3221-4316-9f7e-87683cd2b170")
    /**
     * URL of the package (DEB, RPM, etc.) that contains an update.
     */
    virtual const char* url() const = 0;

    /**
     * Description of changes in this update.
     */
    virtual const char* description() const = 0;

    virtual void apply() = 0;

    virtual Update* copy() const = 0;
};


DECLARE_ZDK_INTERFACE_(Updateable,ZObject)
{
    DECLARE_UUID("7f94b066-e649-4a70-b29d-730fd9658017")

    virtual size_t check_for_updates(EnumCallback<Update*>*) = 0;
};
#endif // UPDATE_H__161E86D5_4E46_4C23_8C4E_D7F211C7FD39
