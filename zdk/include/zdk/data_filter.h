#ifndef DATA_FILTER_H__10CB4CAE_D998_4939_878A_6F46965A5CFC
#define DATA_FILTER_H__10CB4CAE_D998_4939_878A_6F46965A5CFC
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

#include "zdk/enum.h"
#include "zdk/plugin.h"
#include "zdk/variant.h"

struct DebugSymbol;
struct DebugSymbolEvents;
struct Properties;


/**
 * The main purpose of a data filter is to apply transformations
 * on debug symbols in order to present information to the user
 * in the most intuitive way (think STL containers)
 */
DECLARE_ZDK_INTERFACE_(DataFilter, Plugin)
{
    DECLARE_UUID("ee2abe07-14f3-4dab-8c60-55f4953c7f99")

    virtual DebugSymbol* transform( DebugSymbol* symbol,
                                    DebugSymbol* parent,
                                    DebugSymbolEvents*
                                  ) const = 0;

    virtual bool hide(const DebugSymbol*) const = 0;

    /**
     * Each implementation is allowed to define its own "knobs"
     * The callback receives the name, display name, and the default value
     * for each implementation-defined parameter, and returns true if the
     * enumeration should continue or false to stop enumerating.
     * @return the number of enumerated params
     * @note currently only parameters of numeric (integer, double or float)
     * and bool types are supported
     */
    virtual size_t enum_params(
        EnumCallback3<const char*,
                      const char*,
                      const Variant*,
                      bool>*) const = 0;

    /**
     * Signal the plugin that the params have been modified
     * and need to be fetched from the properties object
     * @param name indicate which param has changed (if not NULL)
     */
    virtual void on_param_change(Properties*, const char* name) = 0;

    // virtual void set_param(const char* name, const Variant* value) = 0;

    // virtual void get_param(const char* name, Variant**) const = 0;
};

#endif // DATA_FILTER_H__10CB4CAE_D998_4939_878A_6F46965A5CFC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
