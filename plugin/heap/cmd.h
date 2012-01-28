#ifndef HEAP_COMMAND_H__77F34817_D375_4F42_96CD_8E2FEFFD3CB6
#define HEAP_COMMAND_H__77F34817_D375_4F42_96CD_8E2FEFFD3CB6
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

#include "generic/visitor.h"
#include "zdk/command.h"
#include "zdk/properties.h"
#include "zdk/zobject_impl.h"
#include <string>

class HeapPlugin;
class LinuxLiveTarget;


/**
 * Base class for custom Heap commands
 */
class HeapMonitorCommand
    : public ZObjectImpl<DebuggerCommand>
    , public BaseVisitor
    , public Visitor<LinuxLiveTarget, bool>
{
protected:

BEGIN_INTERFACE_MAP(Macro)
    INTERFACE_ENTRY(DebuggerCommand)
    INTERFACE_ENTRY_DELEGATE(prop_)
END_INTERFACE_MAP()

    HeapMonitorCommand(HeapPlugin*,
                       const std::string& name,
                       const std::string& help,
                       const char* stockID);

    virtual ~HeapMonitorCommand() throw();

    const char* name() const { return name_.c_str(); }

    const char* help() const { return help_.c_str(); }

    void auto_complete(const char*,
        EnumCallback<const char*>*) const
    { }

    bool execute(Thread*, const char* const* argv, Unknown2*);

    Properties& prop() { assert(prop_); return *prop_; }

    /**
     * @note thread is valid only for the duration of the
     * command execution
     */
    Thread* thread() { return thread_.get(); }

    const char* const* args() const { return args_; }

protected:
    HeapPlugin* plugin_;
    RefPtr<Properties> prop_;

private:
    std::string name_;
    std::string help_;
    RefPtr<Thread> thread_;
    const char* const* args_;
};


class HeapMonitorOn : public HeapMonitorCommand
{
public:
    explicit HeapMonitorOn(HeapPlugin*);
    ~HeapMonitorOn() throw () { }

private:
    bool visit(LinuxLiveTarget&);
};


class HeapMonitorOff : public HeapMonitorCommand
{
public:
    explicit HeapMonitorOff(HeapPlugin*);
     ~HeapMonitorOff() throw() { }

private:
    bool visit(LinuxLiveTarget&);
};


class HeapMonitorClear : public HeapMonitorCommand
{
public:
    explicit HeapMonitorClear(HeapPlugin*);
     ~HeapMonitorClear() throw() { }

private:
    bool visit(LinuxLiveTarget&);
};


#endif // HEAP_COMMAND_H__77F34817_D375_4F42_96CD_8E2FEFFD3CB6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
