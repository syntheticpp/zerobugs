#ifndef PythonGate_H__70E93067_9421_4550_8C81_B35381DC19F6
#define PythonGate_H__70E93067_9421_4550_8C81_B35381DC19F6
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

#include <iosfwd>
#include "generic/auto_file.h"
#include "zdk/check_ptr.h"
#include "zdk/priority.h"
#include "zdk/version_info_impl.h"
#include "zdk/update.h"
#include "zdk/weak_ptr.h"
#include "zdk/zero.h"
#include "zdk/zobject_adapt.h"
#include "interp.h"


/**
 * Embeds a python interpreter to which Zero objects are exported
 */
CLASS PythonGate
    : public DebuggerPlugin
    , public VersionInfoImpl<ZERO_API_MAJOR, ZERO_API_MINOR, 32>
    , public Priority
    , public MarshallerCallbacks
{
protected:
    virtual ~PythonGate() throw();

public:
    explicit PythonGate(WeakPtr<Python>&);

    virtual void release();

BEGIN_INTERFACE_MAP(PythonGate)
    INTERFACE_ENTRY(DebuggerPlugin)
    INTERFACE_ENTRY(VersionInfo)
    INTERFACE_ENTRY(Priority)
    INTERFACE_ENTRY_AGGREGATE(adapter_)
    INTERFACE_ENTRY_AGGREGATE(interp_)
    INTERFACE_ENTRY_AGGREGATE(updater_)
END_INTERFACE_MAP()

    Python& interp() { return *CHKPTR(interp_); }

    // --- DebuggerPlugin interface
    /**
     * Pass pointer to debugger and the command line params
     * to plug-in module.
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
     * program.
     */
    virtual void on_detach(Thread*);

    virtual void on_syscall(Thread*, int);

    /**
     * Notification sent to plug-ins when the debuggee stops.
     * The plug-in is given the opportunity to take over
     * handling the user breakpoint. If this method returns
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

    /**
     * When a plug-in calls Debugger::progress, the
     * notification is being passed to all the loaded
     * plug-ins. The initiator may choose to ignore the
     * event if it identifies the cookie as being the
     * same value as passed to the progress() call.
     */
    virtual bool on_progress(const char*, double, word_t);

    bool on_message(const char*, Debugger::MessageType, Thread*, bool)
    { return false; }

    // -- VersionInfo interface
    const char* description() const;

    const char* copyright() const;

    // -- Priority interface
    Priority::Class priority_class() const { return Priority::HIGH; }

    // -- MarshallerCallback interface
    void on_error(const std::string&);

    bool on_command_loop_state(bool);

    void set_grab_event(bool grab);

private:
    /**
     * Entry point for the Python interpreter thread.
     */
    static void* interp_thread(void*);

    void run_interp();

    /**
     * Send a debug event from the main thread to the interpreter thread.
     */
    void send_debug_event(  DebugEvent::Type,
                            const char* name,
                            Thread*,
                            int sysCallNum = -1);

    bool debug_event(RefPtr<DebugEvent>);

private:
    Debugger*           debugger_;
    pthread_t           pyThread_;
    std::string         filename_;  // script passed by command line
    std::string         rcFileName_;
    FILE*               file_;      // script file
    auto_file           rcFile_;
    int                 argc_;
    char**              argv_;
    RefPtr<ZObjectAdapter> adapter_;
    Mutex               mutex_;
    bool                grabEvent_; // true if bypassing internal interp.
    bool                exitOnExcept_;
    RefPtr<Python>      interp_;
    RefPtr<Updateable>  updater_;
};


#endif // PythonGate_H__70E93067_9421_4550_8C81_B35381DC19F6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
