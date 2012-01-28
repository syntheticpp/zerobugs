#ifndef PROXY_TARGET_H__8964835E_009D_4278_AA70_11C4FC356991
#define PROXY_TARGET_H__8964835E_009D_4278_AA70_11C4FC356991
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
#include <string>
#include "dharma/cstream.h"
#include "generic/auto_file.h"
#include "target/linux_live.h"
#include "zdk/xtrace.h"


class RemoteServices;


class ProxyTarget : public LinuxLiveTarget
{
public:
    ProxyTarget(debugger_type&, size_t);
    ~ProxyTarget() throw();

    DECLARE_VISITABLE()

    virtual void init(const char*);

    /**
     * Execute a command (with optional environment) and
     * return the main thread of the newly spawned process
     */
    virtual Thread* exec(const ExecArg&, const char* const* env);

    virtual void read_memory(
        pid_t,
        SegmentType,
        addr_t,
        word_t*,
        size_t,
        size_t*) const;

    bool map_path(std::string&) const;
    const std::string& procfs_root() const;
    std::auto_ptr<std::istream> get_ifstream(const char*) const;

private:
    ObjectFactory& factory() const;

    std::string get_system_release() const;

    RefPtr<SharedString> process_name(pid_t) const;

    const char* id() const { return id_.c_str(); }
    
    virtual VirtualDSO* read_virtual_dso() const;

private:
    mutable auto_fd             sock_;
    mutable CStream             stream_;
    RefPtr<RemoteServices>      xtraceSrv_;
    std::string                 mountPoint_;
    mutable std::string         procRoot_;
    std::string                 arg0_;
    std::string                 id_;
};

#endif // PROXY_TARGET_H__8964835E_009D_4278_AA70_11C4FC356991
