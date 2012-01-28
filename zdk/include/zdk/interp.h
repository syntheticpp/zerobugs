#ifndef INTERP_H__D50BCFEB_7300_4362_AD42_061B734C0AC4
#define INTERP_H__D50BCFEB_7300_4362_AD42_061B734C0AC4
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

#include "zdk/zobject.h"

class Thread;


DECLARE_ZDK_INTERFACE_(InterpreterOutput, RefCounted)
{
    virtual void print(const char*, size_t) = 0;
};

/**
 * A free form interpreter.
 */
DECLARE_ZDK_INTERFACE_(Interpreter, ZObject)
{
    DECLARE_UUID("7a623363-8c57-4824-bd78-02dad0af9831")

    typedef InterpreterOutput Output;

    virtual const char* name() const = 0;

    virtual const char* lang_name() const = 0;

    virtual void run(Thread*, const char*, Output*) = 0;
};

#endif // INTERP_H__D50BCFEB_7300_4362_AD42_061B734C0AC4
