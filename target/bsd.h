#ifndef BSD_H__BE673411_5ABE_11DA_8F2D_00C04F09BBCC
#define BSD_H__BE673411_5ABE_11DA_8F2D_00C04F09BBCC
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

#include "unix.h"


class BSDTarget : public UnixTarget
{
protected:
    explicit BSDTarget(debugger_type&);

public:
    virtual void close_all_files();
};
#endif // BSD_H__BE673411_5ABE_11DA_8F2D_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
