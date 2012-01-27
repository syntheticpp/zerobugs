#ifndef USER_INTERFACE_H__9D4F2D44_5193_4023_9688_2AB5F7BBB634
#define USER_INTERFACE_H__9D4F2D44_5193_4023_9688_2AB5F7BBB634
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/stdexcept.h"
#include "zdk/zero.h"
#include <cassert>
#include <memory>


namespace ui
{
    class Layout;
    class CompositeMenu;


    /**
     * Models the current state of the debugger and UI.
     */
    struct State
    {
        virtual ~State() { }

        /** 
         * NOTE: this method is expected to be called on the
         * main debugger thread only
         */
        virtual void update(Thread*, EventType) = 0;

        /**
         * @return true if target is stopped in the debugger
         */
        virtual bool is_target_stopped() const = 0;

        virtual void set_target_stopped(bool) = 0;

        virtual EventType current_event_type() const = 0;
        virtual Thread* current_thread() const = 0;
    };

   
    class View
    {
    public:
        virtual ~View() { }

        virtual void update(const State&) = 0;

        // visitor pattern
        virtual void accept(Layout&) = 0;
    };

    
    /**
     * Composite view that manages the layout of other views.
     */
    class Layout : public View
    {
    public:
        virtual void add(View&) = 0;
        virtual void show(View&, bool) = 0;
    };


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
    class Command
    {
    public:
        virtual ~Command() { }

        // request from ui to main thread
        virtual void exec_on_main_thread() = 0;

        // response to ui thread
        virtual void exec_on_ui_thread() = 0;
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

        virtual void run();

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

    protected:
        Debugger* debugger() { assert(debugger_); return debugger_; }

        // create an object to hold state
        virtual State* init_state();

        // these are called from the ui thread at the top of run()
        virtual void            init_main_window();
        virtual CompositeMenu*  init_menu();
        virtual Layout*         init_layout();

        virtual int wait_for_event() { return 0; }

    private:
        Debugger*                   debugger_;
        pthread_t                   uiThreadId_;

        std::unique_ptr<State>      state_;
        std::unique_ptr<Layout>     layout_;

        RefPtr<CompositeMenu>       menu_;

        // mail box for passing requests between main and ui threads
        std::unique_ptr<Command>    command_;    
    };

} // namespace 

#endif // USER_INTERFACE_H__9D4F2D44_5193_4023_9688_2AB5F7BBB634
