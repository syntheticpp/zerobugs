#ifndef GUI_H__3A381A36_5089_474B_A1AD_F3A4ADCA433D
#define GUI_H__3A381A36_5089_474B_A1AD_F3A4ADCA433D
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
#include <pthread.h>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include "assert_thread.h"
#include "zdk/command.h"
#include "zdk/export.h"
#include "zdk/priority.h"
#include "zdk/zero.h"
#include "zdk/version_info_impl.h"
#include "zdk/eventlog.h"


#if DEBUG
 #define dbgout(x) if(GUI::debug_level() <= (x)); else std::clog
#else
 using namespace eventlog;
 #define dbgout(level) while (0) eventlog::Null<>()
#endif


class MainWindow;


class ZDK_LOCAL GUI
    : public DebuggerPlugin
    , public CommandCenter
    , public Priority
    , public VersionInfoImpl<ZERO_API_MAJOR, ZERO_API_MINOR, 109>
    , private boost::noncopyable
{
public:
    static GUI* instance();

    static pthread_t tid() { return tid_; }

    static int debug_level() { return debugLevel_; }

    static void handle_exception() throw();

protected:
    GUI();

    virtual ~GUI() throw();

    DESCRIPTION("User Interface Plug-in");

BEGIN_INTERFACE_MAP(GUI)
    INTERFACE_ENTRY(CommandCenter)
    INTERFACE_ENTRY(DebuggerPlugin)
    INTERFACE_ENTRY(Priority)
    INTERFACE_ENTRY(VersionInfo)
END_INTERFACE_MAP()
    //
    // Plugin interface
    //
    void release();
    //
    // DebuggerPlugin interface
    //
    bool initialize(Debugger*, int* argc, char*** argv);

    void start();
    void shutdown();

    void register_streamable_objects(ObjectFactory*);

    void on_table_init(SymbolTable*);

    void on_table_done(SymbolTable*);

    void on_attach(Thread*);

    void on_detach(Thread*);

    bool on_event(Thread*, EventType);

    void on_program_resumed();

    void on_insert_breakpoint(volatile BreakPoint*);

    void on_remove_breakpoint(volatile BreakPoint*);

    bool on_progress(const char*, double, word_t);

    void on_syscall(Thread*, int sysCallNum) { };

    bool on_message(const char*, Debugger::MessageType, Thread*, bool);

    // CommandCenter interface
    void add_command(DebuggerCommand*);
    void enable_command(DebuggerCommand*, bool);

    // Priority interface
    /**
     * The UI takes over the on_event() notification.
     * To ensure that other plug-ins see the events,
     * this plugin is given a LOW priority.
     */
    Priority::Class priority_class() const { return Priority::LOW; }

private:
    /**
     * Start GUI thread. GUI is implemented using
     * gtk-1.2/gtk-2.x and the gtkmm C++ wrappers
     */
    static void* run(void*);

    static void critical_handler(int);

    static ZObject* new_drop_list(ObjectFactory*, const char*);

private:
    static GUI* theGUI_;    // global GUI instance

    static pthread_t tid_;  // thread id

    static int debugLevel_;

    bool attached_;

    boost::shared_ptr<MainWindow> mainWindow_;

    bool disabled_;

    ObjectFactory* factory_;
};


#endif // GUI_H__3A381A36_5089_474B_A1AD_F3A4ADCA433D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
