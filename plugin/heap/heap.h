#ifndef HEAP_H__70E93067_9421_4550_8C81_B35381DC19F6
#define HEAP_H__70E93067_9421_4550_8C81_B35381DC19F6
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

#include "dharma/hash_map.h"
#include "zdk/heap.h"
#include "zdk/priority.h"
#include "zdk/zero.h"
#include "zdk/version_info_impl.h"


class HeapBlockImpl;
class HeapPlugin;

typedef void (HeapPlugin::*Callback)(Thread*, BreakPoint*);


/**
 * This plugin keeps track of the heap-allocated memory,
 * by setting special breakpoints on malloc & friends.
 */
class ZDK_LOCAL HeapPlugin
    : public DebuggerPlugin
    , public Heap
    , public VersionInfoImpl<ZERO_API_MAJOR, ZERO_API_MINOR, 1>
    , public Priority
{
    //typedef std::map<addr_t, RefPtr<HeapBlockImpl> > BlocksByAddr;
    //typedef std::multimap<size_t, RefPtr<HeapBlockImpl> > BlocksBySize;
    typedef ext::hash_map<addr_t, RefPtr<HeapBlockImpl> > BlocksByAddr;
    typedef ext::hash_multimap<size_t, RefPtr<HeapBlockImpl> > BlocksBySize;

protected:
    virtual ~HeapPlugin() throw();

public:
    HeapPlugin();

    void enable(bool);

    virtual void release();

BEGIN_INTERFACE_MAP(HeapPlugin)
    INTERFACE_ENTRY(DebuggerPlugin)
    INTERFACE_ENTRY(Heap)
    INTERFACE_ENTRY(Priority)
    INTERFACE_ENTRY(VersionInfo)
END_INTERFACE_MAP()

    /*** DebuggerPlugin interface ***/
    /**
     * Pass pointer to debugger and the command line params to plug-in.
     */
    virtual bool initialize(Debugger*, int* argc, char*** argv);

    virtual void start();
    virtual void shutdown();

    virtual void register_streamable_objects(ObjectFactory*);

    /**
     * This method is called whenever the debugger
     * is about to read a new symbol table.
     */
    virtual void on_table_init(SymbolTable*);

    /**
     * Called when the debugger has finished reading
     * a symbol table.
     */
    virtual void on_table_done(SymbolTable*);

    /**
     * Called when the debugger attaches itself to a new thread
     */
    virtual void on_attach(Thread*);

    /**
     * Called for each thread that finishes, and with a
     * NULL thread when the debugger detaches from the
     * program (i.e. after detaching from all threads).
     */
    virtual void on_detach(Thread*);

    /**
     * Notification sent to plug-ins when the debuggee stops.
     * The plug-in is given the opportunity to take over
     * handling the user break point. If this method returns
     * true, the notification is not passed around to other
     * plug-ins, and the internal handling is skipped.
     * This way, a plug-in that implements a GUI can take
     * control over the user interaction.
     */
    virtual bool on_event(Thread*, EventType);

    /**
     * Plug-in is notified that the debugger is about to resume
     * all threads in the debugged program.
     */
    virtual void on_program_resumed();

    virtual void on_insert_breakpoint(volatile BreakPoint*);

    virtual void on_remove_breakpoint(volatile BreakPoint*);

    virtual bool on_progress(const char*, double, word_t);

    virtual void on_critical_error(Thread*, const char*) {}

    /*** Heap interface ***/
    virtual size_t enum_blocks(
            EnumCallback<const HeapBlock*>*,
            EnumOption = NONE);

    virtual size_t total() const;

    /*** VersionInfo ***/
    const char* description() const
    {
        return "HeapTracker Plugin";
    }

    const char* copyright() const
    {
        return "Copyright (c) 2006, 2008 Cristian Vlasceanu";
    }

    /*** Priority ***/
    Priority::Class priority_class() const { return Priority::LOW; }

private:
    void set_breakpoint(SymbolTable&, const char*, Callback);

    void set_breakpoint(Thread&, addr_t, Callback, bool temp = false, word_t = 0);

    void on_calloc(Thread*, BreakPoint*);

    void on_calloc_return(Thread*, BreakPoint*);

    void on_malloc(Thread*, BreakPoint*);

    void on_malloc_return(Thread*, BreakPoint*);

    void on_realloc(Thread*, BreakPoint*);

    void on_realloc_return(Thread*, BreakPoint*);

    void on_free(Thread*, BreakPoint*);

    void on_free_return(Thread*, BreakPoint*);

    void add_heap_block(Thread&, addr_t, size_t);

    void remove_heap_block(addr_t);

    void on_syscall(Thread*, int sysCallNum) { };

    bool on_message(const char*, Debugger::MessageType, Thread*, bool)
    { return false; }

    void add_commands();

private:
    Debugger*       debugger_;

    BlocksByAddr    blocksByAddr_;
    BlocksBySize    blocksBySize_;

    size_t          total_;
    int             debugLevel_;
    bool            enabled_;

    RefPtr<DebuggerCommand> on_, off_;
};


#endif // HEAP_H__70E93067_9421_4550_8C81_B35381DC19F6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
