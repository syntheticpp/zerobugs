#ifndef GENERIC_PLUGIN_H__402DA102_6391_4E9F_8D67_AF21155182A6
#define GENERIC_PLUGIN_H__402DA102_6391_4E9F_8D67_AF21155182A6
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

#include "zdk/plugin.h"

struct Debugger;

/**
 * Lightweight plugin base
 */
DECLARE_ZDK_INTERFACE_(GenericPlugin, Plugin)
{
    DECLARE_UUID("daa8a453-132c-4566-9ab3-448f66856b42")

    /**
     * Initialize plugin with pointer to debugger and cmd line.
     */
    virtual bool initialize(Debugger*, int* argc, char*** argv) = 0;

    /**
     * Some plug-ins may require an explicit start
     */
    virtual void start() = 0;

    /**
     * indicates to the plugin that it is about to be
     * unloaded, and it should gracefully release resources
     */
    virtual void shutdown() = 0;
};
#endif // GENERIC_PLUGIN_H__402DA102_6391_4E9F_8D67_AF21155182A6
