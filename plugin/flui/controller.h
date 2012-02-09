#ifndef USER_INTERFACE_H__9D4F2D44_5193_4023_9688_2AB5F7BBB634
#define USER_INTERFACE_H__9D4F2D44_5193_4023_9688_2AB5F7BBB634
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/zero.h"
#include "view.h"
#include <cassert>
#include <memory>


namespace ui
{
    class CodeView;
    class CompositeMenu;
    class Controller;
    class VarView;
   
    /**
     * The UI typically runs an "event-pump" on a separate thread
     * that the main debugger thread (which attaches to targets
     * and manipulates them via ptrace; all ptrace calls have to
     * be issued by the main thread). 
     * The UI may issue a "command" to the main thread, which will
     * execute it and possibly continue with another action on the
     * UI thread. 
     * @see Controller::on_event for how this class is used.
     */
    class Command : public RefCountedImpl<>
    {
    protected:
        virtual ~Command() throw() { }
        
    public:
        // request from ui to main thread
        virtual void execute_on_main_thread() { }
        // response to ui thread
        virtual void continue_on_ui_thread(Controller&) { }

        virtual void cancel() { }

        virtual bool is_done() const { return true; }
    };


    /**
     * Toolkit-agnostic controller. Implements the DebuggerPlugin interface.
     */
    class Controller : public DebuggerPlugin
    {
    public:
        class LockedScope
        {
        public:
            LockedScope(Controller& ui) : ui_(ui) { ui_.lock(); }
            ~LockedScope() { ui_.unlock(); }

        private:
            // non-copyable, non-assignable
            LockedScope(const LockedScope&);
            LockedScope& operator=(const LockedScope&);

            Controller& ui_;
        };

        Controller();
        virtual ~Controller();

        virtual void lock() { }
        virtual void unlock() { }
        virtual void notify_ui_thread() { }

        /**
         * Display an error message (possibly in a message box)
         */
        virtual void error_message(const std::string&) const;

        // UI thread entry point
        virtual void run();

        // main window position and dimensions
        virtual int x() const = 0;
        virtual int y() const = 0;
        virtual int w() const = 0;
        virtual int h() const = 0;

        static void* run(void*);

        // schedule command for execution on main thread
        void call_async_on_main_thread(RefPtr<Command>);

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
        virtual bool on_progress(
            const char* what, 
            double      percent, 
            word_t      cookie);

        virtual bool on_message(
            const char*, 
            Debugger::MessageType, 
            Thread*, 
            bool        /* async */);

        Debugger* debugger() { assert(debugger_); return debugger_; }

    protected:
        void build();
        void build_layout();
        void build_menu();

        // create an object to hold state
        virtual std::unique_ptr<State> init_state();

        // These are called from build(). The initXYZ are factory
        // methods, and the actual widgets that correspond to menu,
        // layout, etc. are "populated" by the buildXYZ methods.
        virtual RefPtr<CodeView>       init_code_view();
        virtual RefPtr<CompositeMenu>  init_menu();
        virtual RefPtr<Layout>         init_layout() = 0;
        virtual RefPtr<VarView>        init_locals_view();

        // this creates the main "application window"
        virtual void            init_main_window();

        virtual int wait_for_event() = 0;

    private:
        RefPtr<Command> update(Thread*, EventType);

    private:
        Debugger*                   debugger_;
        pthread_t                   uiThreadId_;

        RefPtr<Layout>              layout_;
        RefPtr<CompositeMenu>       menu_;
        std::unique_ptr<State>      state_;

        bool                        done_;

        // mail box for passing requests between main and ui threads
        RefPtr<Command>             command_;    
    };

} // namespace 

#endif // USER_INTERFACE_H__9D4F2D44_5193_4023_9688_2AB5F7BBB634
