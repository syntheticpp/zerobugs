#ifndef AutoTest_H__70E93067_9421_4550_8C81_B35381DC19F6
#define AutoTest_H__70E93067_9421_4550_8C81_B35381DC19F6
//
// $Id: autotest.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/priority.h"
#include "zdk/zero.h"
#include "command.h"
#include <deque>
#include <fstream>
#include <map>

/*\!
 This plugins runs automated tests on the engine.
 It reads scripts specified with the '--test=<file>'
 command line options. Script example:
 \!code
    echo *** Test 1 ***
    echo This line is copied verbatim to output
    call help
    call { help show }
    call { show breakpoints }
    expect { }
    call quit
 \!endcode
 */
class AutoTest : public DebuggerPlugin
               , public Priority
               , public std::ios::Init
{
protected:
    virtual ~AutoTest() throw();

public:
    AutoTest();

    virtual void release();

BEGIN_INTERFACE_MAP(AutoTest)
    INTERFACE_ENTRY(DebuggerPlugin)
    INTERFACE_ENTRY(Priority)
END_INTERFACE_MAP()

    /**
     * Pass pointer to debugger and the command line params
     * to plug-in module.
     */
    virtual bool initialize(Debugger*, int* argc, char*** argv);

    virtual void start() { } // no explicit start needed
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

    /**
     * When a plug-in calls Debugger::progress, the
     * notification is being passed to all the loaded
     * plug-ins. The initiator may choose to ignore the
     * event if it identifies the cookie as being the
     * same value as passed to the progress() call.
     */
    virtual bool on_progress(const char*, double, word_t cookie);

    virtual void on_syscall(Thread*, int) { }

    virtual bool on_message(const char*, Debugger::MessageType, Thread*, bool);

    Priority::Class priority_class() const { return Priority::LOW; }

private:
    /* read and parse a script file */
    void read_script(const std::string& filename);

    void print_stats();

private:
    typedef std::deque<boost::shared_ptr<Command> > Commands;

    /* For assigning the result of a command to a "variable": */
    typedef std::map<std::string, std::string> Variables;

    Debugger* debugger_;

    Commands commands_;
    Variables vars_;

    size_t passedCount_, execCount_, cmdCount_;

    std::ofstream log_;
    std::string output_;
    bool debug_;
    bool verbose_;
};


#endif // AutoTest_H__70E93067_9421_4550_8C81_B35381DC19F6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
