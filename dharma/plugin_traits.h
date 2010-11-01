#ifndef PLUGIN_TRAITS_H__C0F17ED9_D743_460F_B135_03506DD21C64
#define PLUGIN_TRAITS_H__C0F17ED9_D743_460F_B135_03506DD21C64
//
// $Id: plugin_traits.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/plugin.h"
#include "dynamic_lib.h"


struct PluginPtrTraits
{
    static void dispose(Plugin* p)
    {
        if (p)
        {
            p->release();
        }
    }
};


template<typename T>
struct NoDisposePtrTraits
{
    static void dispose(T) {} // no-op
};


template<typename T, bool isFunction>
struct IsPointerToPlugin
{
    enum { value = ::boost::is_same<Plugin, T>::value
                || ::boost::is_base_and_derived<Plugin, T>::value
    };
};


template<typename T>
struct IsPointerToPlugin<T, true>
{
    enum { value = 0 };
};


/**
 * Specialization of ImportPtrTraits:
 * PluginPtrTraits is selected as base
 * for T rooted in Plugin
 */
template<typename T>
struct ImportPtrTraits<T*> : public
    TypeSelect<
        IsPointerToPlugin<T,
            ::boost::is_function<T>::value>::value,
        PluginPtrTraits,
        NoDisposePtrTraits<T*>
    >::type
{
    static T* null_value() { return 0; }
};


#endif // PLUGIN_TRAITS_H__C0F17ED9_D743_460F_B135_03506DD21C64
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
