#ifndef HISTORY_H__8373646B_BB66_44BE_9EC8_AF9D765D3EB8
#define HISTORY_H__8373646B_BB66_44BE_9EC8_AF9D765D3EB8
//
// $Id: history.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <deque>
#include <map>
#include <set>
#include <string>
#include "module.h"
#include "zdk/history.h"
#include "zdk/persistent.h"
#include "zdk/watch.h"
#include "zdk/zobject_impl.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "dharma/sarray.h"
#include "dharma/settings.h"


/**
 * A list of variables to watch for, associated with a debug history entry
 */
CLASS WatchListImpl : public ZObjectImpl<WatchList>
{
    std::set<std::string> watches_;

public:
    BEGIN_INTERFACE_MAP(WatchListImpl)
        INTERFACE_ENTRY(WatchList)
    END_INTERFACE_MAP()

    typedef std::set<std::string>::const_iterator const_iterator;

    ~WatchListImpl() throw() { }

    const_iterator begin() const { return watches_.begin(); }
    const_iterator end() const { return watches_.end(); }

    bool add(const char* watch);
    void clear();
    size_t foreach(EnumCallback<const char*>*) const;
};


/**
 * An entry in the history of recently debugged programs
 */
CLASS HistoryEntryImpl : public Persistent, public ZObjectImpl<HistoryEntry>
{
public:
    // modules, keyed by name
    typedef ext::hash_map<RefPtr<SharedString>, RefPtr<ModuleImpl> > Modules;
    typedef Modules::const_iterator const_iterator;

    BEGIN_INTERFACE_MAP(HistoryEntryImpl)
        INTERFACE_ENTRY(HistoryEntry)
        INTERFACE_ENTRY_INHERIT(Persistent)
        INTERFACE_ENTRY_AGGREGATE(watches_)
        INTERFACE_ENTRY_DELEGATE(prop_)
    END_INTERFACE_MAP()

    HistoryEntryImpl(WeakPtr<ObjectFactory>, const char*);

    explicit HistoryEntryImpl(Process&);

    virtual ~HistoryEntryImpl() throw();

    pid_t pid() const { return pid_; }

    const char* name() const { return name_.c_str(); }

    time_t last_debugged() const { return lastDebugged_; }

    const char* command_line() const;
    void set_command_line(const char*);

    const char* const* environ() const  { return env_.cstrings(); }
    void set_environ(const char* const* env);

    bool is_live() const { return isLive_; }

    const char* target_param() const { return targetParam_.c_str(); }

    ProcessOrigin origin() const { return origin_; }

    // Module methods -- used internally by DebuggerBase
    void add_module(const RefPtr<ModuleImpl>&);

    /**
     * lookup module by name
     */
    RefPtr<ModuleImpl> get_module(const RefPtr<SharedString>&) const;

    /**
     * Lookup module by address: if address falls into the range
     * of addresses for one of the modules in this entry, return it.
     */
    RefPtr<ModuleImpl> get_module(addr_t) const;

    ///// InputStreamEvents Interface /////
    void on_word(const char*, word_t);

    void on_string(const char*, const char*);

    size_t on_object_begin(InputStream*, const char*, uuidref_t, size_t);

    ///// Streamable Interface /////
    uuidref_t uuid() const { return HistoryEntry::_uuid(); }

    size_t write(OutputStream*) const;

    /**
     * Set the path under which to store process-specific stuff and
     * other info that does not have to be in the config file.
     */
    void set_path(const std::string& path);

    void read_environ();

    void destroy();

    const_iterator modules_begin() const { return modules_.begin(); }
    const_iterator modules_end() const { return modules_.end(); }

    const std::string& path() const { return path_; }

private:
    std::string         cmdLine_;
    pid_t               pid_;
    time_t              lastDebugged_;
    bool                isLive_;
    std::string         name_;
    Modules             modules_;
    std::string         path_;
    std::string         targetParam_;
    SArray              env_;
    ProcessOrigin       origin_;
    RefPtr<Settings>    prop_; // more properties
    RefPtr<WatchListImpl>  watches_;
    WeakPtr<ObjectFactory> factory_;
};


/**
 * The history of recently debugged programs
 */
CLASS HistoryImpl : public Persistent, public ZObjectImpl<History>
{
    typedef std::deque<RefPtr<HistoryEntryImpl> > Queue;

public:
    DECLARE_UUID("78d2d5ba-4228-4963-a551-c4110327129f")
BEGIN_INTERFACE_MAP(HistoryImpl)
    INTERFACE_ENTRY(History)
    INTERFACE_ENTRY(HistoryImpl)
    INTERFACE_ENTRY_INHERIT(Persistent)
END_INTERFACE_MAP()

    HistoryImpl(WeakPtr<ObjectFactory>, const std::string& path);

    virtual ~HistoryImpl() throw();

    size_t entry_count() const { return queue_.size(); }

    const HistoryEntry* entry(size_t) const;

    void remove_entry(size_t);

    static ZObject* create(ObjectFactory* factory, const char* name)
    { return new HistoryImpl(factory, name); }

    void take_snapshot(Debugger&);

    RefPtr<HistoryEntryImpl> get_entry_impl(const char* name) const;

    HistoryEntry* get_entry(const char* name) const
    {
        return get_entry_impl(name).get();
    }

    // InputStreamEvents
    size_t on_object_begin(InputStream*, const char*, uuidref_t, size_t);

    void on_object_end() { /* do nothing */ }

    void on_string(const char*, const char*);

    // Streamable
    uuidref_t uuid() const { return History::_uuid(); }

protected:
    size_t write(OutputStream*) const;

    void add_entry(Process*);

private:
    Queue           queue_;
    std::string     path_;
    mutable Mutex   mutex_;
    WeakPtr<ObjectFactory> factory_;
};
#endif // HISTORY_H__8373646B_BB66_44BE_9EC8_AF9D765D3EB8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
