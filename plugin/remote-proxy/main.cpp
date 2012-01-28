//
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
#include "proxy-plugin.h"
#include "proxy-target.h"


int32_t query_version(int32_t* minor)
{
    if (minor) *minor = ZDK_MINOR;
    return ZDK_MAJOR;
}


/**
 * Advertise the interfaces supported by this plugin
 */
void query_plugin(InterfaceRegistry* registry)
{
    registry->update(GenericPlugin::_uuid());
}


/**
 * Create a plugin instance
 */
Plugin* create_plugin(uuidref_t iid)
{
    if (uuid_equal(GenericPlugin::_uuid(), iid))
    {
        return new ProxyPlugin;
    }
    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
