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
#include "zdk/config.h"
#include <errno.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <boost/tokenizer.hpp>
#include "directory.h"
#include "plugin_manager.h"
#include "sigutil.h"
#include "system_error.h"

using namespace std;


typedef bool (PluginManager::*Notify)(DynamicLibPtr, uuidref_t, Unknown2*&);

//
// Helper functors
//
namespace
{
    /**
     * Function object for querying a dynamic library for plugins.
     * Implements the InterfaceRegistry interface, for internal use
     */
    class ZDK_LOCAL QueryLibrary : public InterfaceRegistry
    {
    public:
        virtual ~QueryLibrary() {}

        QueryLibrary(PluginManager& mgr, Notify notify)
            : mgr_(mgr), notify_(notify) {}

        void operator()(const string& filename)
        {
            currentLib_ = make_shared<DynamicLib>(filename.c_str());

            bool isCompatibleVersion = true;

            ImportPtr<int32_t (int32_t*)> query_version;
            if (!currentLib_->import("query_version", query_version))
            {
                isCompatibleVersion = false;
            }
            else
            {
                int32_t major = 0, minor = 0;
                major = (*query_version)(&minor);

                if ((major != mgr_.version_major()) || (minor > mgr_.version_minor()))
                {
                    isCompatibleVersion = false;
                }
            }
            if (!isCompatibleVersion)
            {
                cerr << "Incompatible version: " << currentLib_->filename() << endl;
                return;
            }

            ImportPtr<void (InterfaceRegistry*)> query;

            // import a function named "query_plugin"; if successful, invoke it
            if (currentLib_->import("query_plugin", query))
            {
                (*query)(this);
            }
        }

    private:
        bool update(uuidref_t iid)
        {
            Unknown2* component = NULL;
            return (mgr_.*notify_)(currentLib_, iid, component);
        }

        DynamicLibPtr   currentLib_;
        PluginManager&  mgr_;
        Notify          notify_;
    };


    /**
     * Function object for searching plugins in a given directory
     */
    class ZDK_LOCAL DirectoryScan
    {
    public:
        DirectoryScan(PluginManager& mgr, Notify notify)
            : mgr_(mgr)
            , notify_(notify)
        {}

        void operator()(const string& path) const
        {
            try
            {
                Directory dir(path, "*.so");

                QueryLibrary query(mgr_, notify_);
                // inspect all .so files in the directory
           #ifndef HAVE_LAMBDA_SUPPORT
                for_each<Directory::const_iterator, QueryLibrary&>(
                    dir.begin(), dir.end(), query);
           #else
                for_each(dir.begin(), dir.end(), [&query](const string& d) {
                    query(d);
                });
           #endif
            }
            catch (const SystemError& e)
            {
                // silently ignore if no such path
                if (e.error() != ENOENT)
                {
                    throw;
                }
            }
        }

    private:
        PluginManager&  mgr_;
        Notify          notify_;
    };
} // namespace


////////////////////////////////////////////////////////////////
PluginManager::PluginManager(int major, int minor)
    : major_(major)
    , minor_(minor)
{
}


////////////////////////////////////////////////////////////////
PluginManager::~PluginManager()
{
    pluginMap_.clear();
}


////////////////////////////////////////////////////////////////
void PluginManager::scan_plugins(const string& path)
{
    if (const char* p = getenv(path.c_str()))
    {
        path_ = p;
    }
    else
    {
        path_ = path;
    }
    scan();
    on_scan_plugins_complete();
}


////////////////////////////////////////////////////////////////
void PluginManager::scan()
{
    typedef boost::char_separator<char> Delim;
    typedef boost::tokenizer<Delim> Tokenizer;

    Tokenizer tok(path_, Delim(":"));

    for_each(tok.begin(), tok.end(),
        DirectoryScan(*this, &PluginManager::on_interface));
}


////////////////////////////////////////////////////////////////
bool
PluginManager::on_interface(DynamicLibPtr lib,
                            uuidref_t iid,
                            Unknown2*&)
{
    assert(lib);
    assert(iid);

    const ZDK_UUID& uuid = *iid;
    Range range = pluginMap_.equal_range(uuid);

    for (const_iterator i = range.first; i != range.second; ++i)
    {
        if (lib->filename() == i->second->filename())
        {
            return false;
        }
    }
    pluginMap_.insert(make_pair(uuid, lib));
    return true;
}


////////////////////////////////////////////////////////////////
ImportPtr<Plugin>
PluginManager::create_instance(DynamicLibPtr lib, uuidref_t uuid)
{
    ImportPtr<Plugin> plugin;
    ImportPtr<Plugin* (uuidref_t)> create;

    if (lib->import("create_plugin", create))
    {
        plugin = (*create)(uuid);
    }
    return plugin;
}


////////////////////////////////////////////////////////////////
void PluginManager::unload_unreferenced()
{
    PluginMap::iterator i = pluginMap_.begin();
    while (i != pluginMap_.end())
    {
        DynamicLibPtr lib = i->second;

        if (lib->count() == 1)
        {
            pluginMap_.erase(i++);
        }
        else
        {
            ++i;
        }
    }
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
