#ifndef PROXY_H__316985BA_7EAA_4C80_A699_0A637722FBF1
#define PROXY_H__316985BA_7EAA_4C80_A699_0A637722FBF1
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
#include "zdk/export.h"
#include "zdk/generic_plugin.h"
#include "zdk/version_info_impl.h"
#include "zdk/zobject_impl.h"
#include <map>
#include <string>

class DebuggerBase;
class Target;

typedef std::map<std::string,
                 RefPtr<Target>(*)(DebuggerBase&) > TargetFactoryMap;

class ZDK_LOCAL ProxyPlugin
    : public ZObjectImpl<GenericPlugin>
    , public VersionInfoImpl<ZERO_API_MAJOR, ZERO_API_MINOR, 1>
{
    DECLARE_UUID("8540cad7-a56b-49a1-961e-a78d0f487b6b")
    DESCRIPTION("Remote Debugger Proxy")

    Debugger* dbg_;
    TargetFactoryMap factoryMap_;

public:
    ProxyPlugin();
    ~ProxyPlugin() throw();

    BEGIN_INTERFACE_MAP(ProxyPlugin)
        INTERFACE_ENTRY(Plugin)
        INTERFACE_ENTRY(GenericPlugin)
        INTERFACE_ENTRY(VersionInfo)
    END_INTERFACE_MAP()

    bool initialize(Debugger*, int* argc, char*** argv);

    void start();

    void shutdown();

    void release();
};

#endif // PROXY_H__316985BA_7EAA_4C80_A699_0A637722FBF1
