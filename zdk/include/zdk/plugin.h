#ifndef PLUGIN_H__DA4D7589_0FA3_4306_9F15_BA373B488DB7
#define PLUGIN_H__DA4D7589_0FA3_4306_9F15_BA373B488DB7
//
// $Id: plugin.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "unknown2.h"

/**
 * Base plug-in class.
 */
DECLARE_ZDK_INTERFACE_(Plugin, Unknown2)
{
    DECLARE_UUID("d246f217-5a3a-4743-a005-047594e3a5c9")

    virtual void release() = 0;
};


DECLARE_ZDK_INTERFACE(InterfaceRegistry)
{
    virtual bool update(uuidref_t) = 0;
};


//
// These functions need to be exposed by shared libraries
// that implement plug-in components. The C-linkage is required
// because the mangling schemes are compiler-dependent.
//
extern "C"
{
    ZDK_EXPORT void query_plugin(InterfaceRegistry*);

    ZDK_EXPORT Plugin* create_plugin(uuidref_t);

    ZDK_EXPORT int32_t query_version(int32_t* minor);
}

#endif // PLUGIN_H__DA4D7589_0FA3_4306_9F15_BA373B488DB7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
