#ifndef CONTROLLER_H__45E9C688_9B30_4AE2_92FD_5D064B56EFC7
#define CONTROLLER_H__45E9C688_9B30_4AE2_92FD_5D064B56EFC7
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/zero.h"
#include "zdk/plugin.h"
#include "command.h"
#include "view.h"
#include <cassert>
#include <memory>


namespace ui
{
    class CodeView;
    class CompositeMenu;
    class BreakPointView;
    class Dialog;
    class PopupMenu;
    class StackView;
    class Toolbar;
    class VarView;

    enum EnableMode
    {
        Disable,
        Enable,
        Toggle
    };

    /**
     * Controller executes this command when idle.
     */
    class IdleCommand : public Command
    {
    public:
        IdleCommand();

        // break out of idle state
        void cancel();

    protected:
        ~IdleCommand() throw() { }

    private:
        void execute_on_main_thread();
        void set_cancel();

        Mutex       mutex_;
        Condition   cond_;
        bool        cancelled_;
    };


    /**
     * Toolkit-agnostic controller. Implements the DebuggerPlugin interface.
     */
    class Controller : public DebuggerPlugin
    {
        class StateImpl;

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

    public:
        Controller();
        virtual ~Controller();

        virtual void lock() { }
        virtual void unlock() { }
        virtual void notify_ui_thread() { }

        /**
         * Display an error message (possibly in a message box)
         */
        virtual void error_message(const std::string&) = 0;
        virtual void status_message(const std::string&);

        // UI thread entry point
        static void* run(void*);

        virtual void run();

        // main window position and dimensions
        virtual int x() const = 0;
        virtual int y() const = 0;
        virtual int w() const = 0;
        virtual int h() const = 0;

        // *** Dialogs ***
        virtual void show_edit_breakpoint_dialog(addr_t) = 0;
        virtual void show_eval_dialog() = 0;

        // schedule command for execution on main thread
        void call_main_thread_async(RefPtr<Command>);

        // wake up main thread if in idle state
        void awaken_main_thread();

        Debugger* debugger() {
            assert(debugger_);
            return debugger_;
        }

        Dialog* current_dialog() {
            return dialogStack_.empty() ? nullptr : dialogStack_.back();
        }

        void set_current_dialog(Dialog* dialog);

        // --- Breakpoint management
        // insert/remove breakpoint
        void toggle_user_breakpoint();
        void toggle_user_breakpoint(addr_t);

        // enable/disable breakpoint
        void enable_user_breakpoint(addr_t, EnableMode);

        // set temporary breakpoint at given address, for the current thread
        void set_temp_breakpoint(addr_t);
        void set_user_breakpoint(addr_t, bool);

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

        State& state();

        virtual RefPtr<PopupMenu>       init_contextual_menu() = 0;

    protected:
        void build();
        void build_layout();
        void build_menu();
        void build_toolbar();

        void done();

        // create an object to hold state
        virtual std::unique_ptr<StateImpl> init_state();

        // These are called from build(). The initXYZ are factory
        // methods, and the actual widgets that correspond to menu,
        // layout, etc. are "populated" by the buildXYZ methods.

        virtual RefPtr<CodeView>        init_code_view() = 0;
        virtual RefPtr<CompositeMenu>   init_menu() = 0;
        virtual RefPtr<BreakPointView>  init_breakpoint_view() = 0;
        virtual RefPtr<Layout>          init_layout() = 0;
        virtual RefPtr<VarView>         init_locals_view() = 0;
        virtual RefPtr<StackView>       init_stack_view() = 0;
        virtual RefPtr<Toolbar>         init_toolbar() = 0;

        // this creates the main "application window"
        virtual void init_main_window(int x, int y, int w, int h) = 0;

        virtual int wait_for_event() = 0;

        virtual void save_configuration();

    private:
        RefPtr<Command> update(Thread*, EventType);
        void update(LockedScope&, Thread*, EventType);

        bool probe_interactive_plugins();

    private:
        Debugger*                   debugger_;
        pthread_t                   threadId_;      // UI thread ID
        RefPtr<Layout>              layout_;
        RefPtr<View>                breakpoints_;
        RefPtr<CodeView>            code_;
        RefPtr<CompositeMenu>       menu_;
        RefPtr<Toolbar>             toolbar_;
        std::unique_ptr<StateImpl>  state_;

        bool                        done_;          // terminate UI loop?
        bool                        probing_;
        std::vector<Dialog*>        dialogStack_;   // modal dialogs

        // mail box for passing requests between main and ui threads
        RefPtr<Command>             command_;

        RefPtr<IdleCommand>         idle_;
    };

} // namespace


#endif // CONTROLLER_H__45E9C688_9B30_4AE2_92FD_5D064B56EFC7

