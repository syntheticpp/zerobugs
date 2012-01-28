#ifndef PLUGIN_MANAGER_H__1059708842
#define PLUGIN_MANAGER_H__1059708842
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
#include <map>
#include <string>
#include "zdk/plugin.h"
#include "zdk/unknown2.h"
#include "dynamic_lib.h"
#include "plugin_traits.h"


/**
 * Inspects all dynamic libraries in a given list of paths,
 * looking for libraries that implement plug-ins;
 * constructs a map from plugin names to the full path
 * of the dynamic library that implement the plugin.
 */
class ZDK_LOCAL PluginManager : public Unknown
{
public:
    typedef std::multimap<ZDK_UUID, DynamicLibPtr> PluginMap;
    typedef PluginMap::const_iterator const_iterator;
    typedef std::pair<const_iterator, const_iterator> Range;

    PluginManager(int major, int minor);

    virtual ~PluginManager();

    const std::string& path() const { return path_; }

    void scan_plugins(const std::string& path);

    int version_major() const { return major_; }
    int version_minor() const { return minor_; }

protected:
    virtual bool on_interface(DynamicLibPtr, uuidref_t, Unknown2*&);

    virtual void on_scan_plugins_complete() {}

    void unload_unreferenced();

    template<typename T>
    void create_instance(DynamicLibPtr lib, ImportPtr<T>& ptr)
    {
        ImportPtr<Plugin> plugin = create_instance(lib, T::_uuid());
        ptr = import_dynamic_cast<T>(plugin);
    }

private:
    static ImportPtr<Plugin> create_instance(DynamicLibPtr, uuidref_t);

    void scan();

private:
    std::string path_;
    PluginMap   pluginMap_;
    int         major_,
                minor_;
};

#endif // PLUGIN_MANAGER_H__1059708842
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
