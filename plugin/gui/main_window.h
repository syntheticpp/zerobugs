#ifndef MAIN_WINDOW_H__9CEB24F3_F2D9_4E9C_9353_C08697880309
#define MAIN_WINDOW_H__9CEB24F3_F2D9_4E9C_9353_C08697880309
//
// $Id: main_window.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <map>
#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "gtkmm/events.h"
#include "gtkmm/icon_mapper.h"
#include "gtkmm/row_handle.h"
#include "gtkmm/window.h"
#include "zdk/interp.h"
#include "zdk/mutex.h"
#include "zdk/zero.h"
#include "app_slots.h"
#include "layout_manager.h"
#include "right_click.h"
#include "states.h"
#include "view_types.h"


class DataType; // full def in zdk/data_type.h

class HistoryEntry;
class InterpreterBox;
class ExprEvalDialog;
class MemoryRequestHandler;
class MemoryView;
class MenuEntry;
class ProgressBox;
class ProgramToolBar2;
class ProgramView;
class RegisterView;
class SArray;
class StackView;
class ThreadView;
class ToolBar;
class VariablesView;
class WatchView;

struct BreakPointSite; // edit_breakpoint_dlg.h

namespace Gtk
{
    class Box;
    class Entry;
    class FileSelection;
    class HandleBox;
    class Label;
    class Menu;
    class MenuBar;
    class Paned;

    namespace Menu_Helpers
    {
        class MenuList;
    }
}


////////////////////////////////////////////////////////////////
using Gtk::Menu_Helpers::MenuList;


/**
 * The application toplevel window, parent of all other widgets.
 * Marshals calls between the UI thread and the main debugger
 * thread using the responses_ and requests_ command queues.
 */
class ZDK_LOCAL MainWindow
    : public OnMapEventImpl<Gtk::Window>
    , public AppSlots
    , private boost::noncopyable
{
public:
    MainWindow(Debugger&, const std::string& strategy);

    ~MainWindow() throw();

    bool on_window_state_event(GdkEventWindowState* event);
    void on_size_allocate(Gdk::Rectangle& allocation);

    bool on_debug_event(RefPtr<Thread>, EventType) volatile;

    void on_attached(RefPtr<Thread>);

    void on_detached(RefPtr<Thread>);

    void on_insert_breakpoint(RefPtr<Thread>, RefPtr<Symbol>, bool);

    void on_remove_breakpoint(RefPtr<Thread>, RefPtr<Symbol>, bool);

    void on_deferred_breakpoint_removed(Symbol*);

    bool on_progress(const char*, double) volatile;

    void on_thread_finished(RefPtr<Thread>, std::string);

    void on_eval_error(std::string);

    void on_visibility_change(NotebookPtr);

    /**
     * Display an error message.
     */
    void error_message(std::string);
    void error_message_async(const std::string&);

    /**
     * Display an informative message.
     */
    void info_message(std::string);
    void info_message_async(const std::string&);

    void question_message(std::string, bool*, const char* = 0);

    /**
     * Set the text in the bottom area where the name of
     * the current function is being displayed (the area may be
     * used for other status information as well).
     * @note the parameter is passed by value on purpose.
     */
    void status_message(std::string);

    /**
     *  Change the layout strategy at runtime
     */
    void layout_work_area(const std::string& strategy);

    void set_option(uint64_t mask, bool, bool force = false);
    void set_option_mask(uint64_t mask, bool force = false);

    void load_key_bindings(Properties&);
    void save_key_bindings();

    bool add_toolbar(const char* name, ToolBar&, bool visible = true);

    void help_search(std::string);
    void update(RefPtr<Thread>);

private:
    typedef boost::shared_ptr<ProgressBox> ProgressBoxPtr;
    typedef RefPtr<MemoryView> MemoryViewPtr;

    typedef std::multimap<pid_t, MemoryViewPtr> MemoryViews;

    typedef boost::shared_ptr<RegisterView> RegisterViewPtr;
    typedef boost::shared_ptr<StackView> StackViewPtr;
    typedef boost::shared_ptr<ThreadView> ThreadViewPtr;
    typedef boost::shared_ptr<ProgramView> ProgramViewPtr;
    typedef boost::shared_ptr<VariablesView> VariablesViewPtr;
    typedef boost::shared_ptr<WatchView> WatchViewPtr;
    typedef boost::shared_ptr<InterpreterBox> InterpBoxPtr;

    typedef std::map<std::string, ToolBar*> ToolMap;

    /**
     * @note parameter passed by value purposely, so that
     * we can use it with post_request (asynchronously)
     */
    void message_box(const std::string&, const char* [], bool async);
    void message_box(std::string, const char* []);

    void update_var_views();
    void update_thread_view(RefPtr<Thread>);

    /**
     * disable / enable buttons, etc.
     */
    void update_state(size_t stateMask);

    void compute_and_update_state(RefPtr<Thread>, unsigned int = 0);
    void refresh_state() { compute_and_update_state(current_thread()); }

    bool on_invalid_utf8(const char*, size_t);

    void update_status_bar();

    void update_eval_view(bool complete = false) volatile;

    /**
     * print current symbol in the upper status area, bring the
     * frame in view (if not NULL, and still on the stack)
     */
    void show_symbol(RefPtr<Thread>, RefPtr<Symbol>, Frame*);

    void on_symbol_change(RefPtr<Thread>, RefPtr<Symbol>, Frame*);

    /**
     * helper called by show_symbol
     */
    void show_frame(Frame*);

    /**
     * Display the code (as source or assembly) so that
     * the current's frame program counter is in view,
     * and update the status label at the top of the
     * stacktrace view.
     */
    void display_code_and_status(EventType);

    /**
     * bring thread in view
     */
    void set_active_thread(RefPtr<Thread>) /* volatile */;

    /**
     * Popup a message box showing a description of
     * the signal caught by the debugged program,
     * if necessary.
     */
    void message_box_signal();

    /**
     * Verifies if the current thread has exited (either
     * normally or killed by a signal), and if so,
     * a message is presented to the user. Returns true
     * if thread still running, false otherwise.
     */
    bool check_thread(RefPtr<Thread>) volatile;

    event_result_t on_delete_event(GdkEventAny*);

    void create_work_area();
    void layout_work_area();

    Gtk::Widget& create_menu_bar();
    Gtk::Widget& create_status_bar();

    void create_thread_view();
    void create_local_vars_view();
    void create_watches_view();
    void create_interp();
    void create_interp_box(Interpreter*);

    // connect common VariablesView signals
    void connect_var_common(VariablesView&);

    // Connect signals to slots
    void connect_codeview();
    void connect_toolbar();

    void create_menu_elem(Gtk::Menu_Helpers::MenuList&, const MenuEntry&);

    void create_menu_elem(Gtk::Menu&, const MenuEntry* []);

    void do_attach(pid_t, const std::string&);

#if defined (GTKMM_2)
    bool on_request(Glib::IOCondition);

    bool on_percent(Glib::IOCondition);
#else
    // GTK-1.2
    /**
     * Called when there's a request from the main
     * debugger thread pending in the pipe.
     */
    void on_request(int, GdkInputCondition);

    void on_percent(int, GdkInputCondition);
#endif

    /**
     * Called when the user selects an item in the
     * list that shows the current stack trace of
     * a debugged thread.
     */
    void on_stack_selection();

    void on_thread_selection(pid_t, unsigned long);

    static void on_change_state(Gtk::Widget&, size_t stateMask);

    void on_can_navigate_back(size_t);
    void on_can_navigate_forward(size_t);

    // menu slots
    void on_menu_attach();
    void on_menu_detach();
    void on_menu_source();
    void on_menu_close();
    void on_menu_close_all();
    void on_menu_load_core();

    void on_core_selected(Gtk::FileSelection*);

    void on_menu_run();
    void on_menu_quit();

    void on_menu_escape_to_command_line();

    void on_menu_save_stack();
    void on_stack_file_selected(Gtk::FileSelection*);

    /**
     * Popup a dialog prompting the user for a function
     * name or address in the code where to set a breakpoint
     */
    void on_menu_insert_breakpoints();          // menu bar
    void on_menu_edit_breakpoints();            // menu bar
    void on_menu_list_breakpoints();            // menu bar
    void on_menu_clear_all_breakpoints();

    void on_menu_set_breakpoint(RightClickInfo*, bool);
    void on_menu_disable_breakpoint(addr_t, size_t);
    void on_menu_enable_breakpoint(addr_t, size_t);

    /**
     * called when breakpoint at given address is enabled or
     * disabled, so that it gets shown in the correct color
     */
    void update_breakpoint_view(addr_t, size_t line);

    void on_menu_set_program_count(addr_t);     // Set Next Line
    void on_menu_show_next_line(addr_t, bool inUnit);

    void on_menu_evaluate(DebugSymbolList);
    void on_menu_variable();

    void on_menu_edit_signals();
    void on_menu_find();
    void on_menu_find_again();

    void on_menu_lookup_symbol();
    void on_menu_view_type(ViewType);

    void on_menu_stop();
    void on_menu_continue();
    void on_menu_next();
    void on_menu_return();
    void on_menu_step();
    void on_menu_instruction();
    void on_menu_restart();
    void on_menu_heap();
    void on_menu_history();
    void on_menu_fonts();
    void on_menu_toggle_toolbar(ToolBar*);
    void on_menu_view_modules();

    void popup_memory_window(addr_t);

    /**
     * This method handles the "View Memory" menu entry.
     */
    void on_menu_memory() { popup_memory_window(0); }

    /**
     * Handles text search failures.
     */
    bool on_not_found(const std::string&);

    void on_menu_about();
    void on_menu_check_for_updates();
    void on_menu_help();
    void on_help(const char* searchTopic = NULL);

    // slots for watchpoint management
    void on_menu_watch();
    void on_menu_watchpoint(RefPtr<DebugSymbol>);
    void on_mem_watch(WatchType, addr_t);
    void on_value_watch(RelType, const std::string&, RefPtr<DebugSymbol>);

    void on_menu_edit_watchpoints();
    void on_menu_edit_options();

    void on_menu_toggle_break_on_throw();

    void on_run(const std::string&);
    void on_execute(const HistoryEntry*);

    void on_menu_manage_step_over(RefPtr<Symbol>);

    /**
     * This slot writes a value to a CPU register of the
     * specified task (thread) on behalf of another widget.
     */
    void on_set_reg(RefPtr<Register>, std::string, std::string);

    void on_describe_type(RefPtr<DebugSymbol>, int);
    void on_what_is(RefPtr<DataType>);

    bool on_variable_edit(RefPtr<DebugSymbol>, const std::string&);

    void on_symbol_expand(DebugSymbol*);

    /**
     * Lookup named symbols in the context of a given code address
     * and populates the DebugSymbolList with the results.
     */
    void on_query_symbols(std::string, addr_t, DebugSymbolList*, bool);

    bool on_evaluate(std::string, addr_t, ExprEvents*, int base);

    void on_refresh(VariablesView*);

    /**
     * The UI thread cannot call directly into debugger functions
     * that perform a ptrace() call internally, since only the thread
     * that did a PTRACE_ATTACH can do so. This method posts a command
     * (as if typed at a command line prompt) from the UI thread to the
     * main debugger thread. If the file descriptor fd is valid (i.e.
     * not negative) then the output of the command is redirected to
     * that file.
     */
    void post_debugger_command(const std::string&, int fd = -1);

    void on_source_selected(Gtk::FileSelection*);

    /**
     * Instructs the debugger engine to insert user breakpoints
     * at each of the addresses in the vector. The vector is passed
     * by value because this call is marshalled between threads.
     * @param silent if true, don't show a message box when complete
     * @param permanent if false, it is just a temporary breakpoint
     * (as in the Run To Cursor scenario).
     * @note: this function runs on the main thread
     */
    void insert_breakpoints
     (
        RefPtr<Thread>,
        std::vector<addr_t>,
        bool silent,
        bool permanent
     );

    bool insert_breakpoints_and_resume
     (
        RefPtr<Thread>,
        std::vector<addr_t>,
        bool permanent
     );
    void insert_deferred_breakpoints
     (
        RefPtr<Thread>,
        std::vector<RefPtr<Symbol> >,
        bool silent,
        bool permanent
     );

    bool insert_deferred_breakpoints_and_resume
     (
        RefPtr<Thread>,
        std::vector<RefPtr<Symbol> >,
        bool permanent
     );

    void report_breakpoints_inserted(std::ostringstream&, size_t count);

    /**
     * The breakpoint specification string can be an address
     * or a function name
     */
    void set_breakpoint_on_main_thread(std::string, RefPtr<Thread>);
    void set_breakpoints_on_ui_thread(std::string,
                                     RefPtr<Thread>,
                                     std::vector<RefPtr<Symbol> >);

    /**
     * Delete the user breakpoints (if any exist) at each of the
     * addresses in the vector.
     * @note: runs on the main thread.
     * @note the vector is passed in by value so that we can safely
     * marshal a call to this method from one thread to another.
     */
    void delete_breakpoints(std::vector<BreakPointSite>);

    void on_delete_breakpoints(const std::vector<BreakPointSite>&);

    /**
     * Execute one line of code, or one machine
     * instruction (if a disassembly window),
     * stepping over function calls.
     */
    void next(Debugger::StepMode);

    /**
     * Helper, invoked by next() on main thread
     */
    void step_to(Debugger::StepMode, addr_t, addr_t);

public:
    /**
     * Updates the UI state to reflect that the
     * debugged program is running.
     */
    void update_running();

    void clear_all_views();

    /**
     * Add user-defined, custom command
     */
    void add_command(DebuggerCommand*);
    void enable_command(DebuggerCommand*, bool);

    ProgramView* program_view() { return progView_.get(); }

    void apply_font(const std::string&);

private:
    // toolbar customization
    // todo: one day I should add support for customizing
    //  the UI using XUL (http://www.mozilla.org/projects/xul/)
    ToolBar* get_toolbar(Properties&);

    void toolbar_entry_dialog(DebuggerCommand*, Properties*);

    /**
     * for user-defined toolbar buttons
     */
    void add_toolbar_button(DebuggerCommand*,
                            const std::string&,
                            const char* stock,
                            SArray& pixmap,
                            Properties&);

    void add_toolbar_dialog(DebuggerCommand*,
                            const std::string&,
                            const char* stock,
                            SArray& pixmap,
                            Properties&);

    void run_macro(DebuggerCommand*, const SArray&);

    /**
     * Updates the main window state, called when some
     * event caused the debugged program to stop.
     * @note called on the UI thread via requests_ queue.
     */
    void update_display(EventType);

    /**
     * Called by on_debug_event
     */
    bool process_debug_event(const RefPtr<Thread>&, EventType) volatile;

    /**
     * Waits for a response from the UI thread,
     * and executes it.
     */
    void wait_and_process_responses() volatile;

    /**
     * Shows how much has completed of an operation.
     * @note msg string purposely passed in by value
     * so that a call to this function may be safely
     * marshaled from the main thread to the UI thread
     */
    void show_progress_indicator(std::string msg, double percent);

    void handle_error(const char* func, const std::string& msg) volatile;

    void update_after_responses();

    /**
     * Write current app window state to debugger properties.
     * @see the Properties interface.
     */
    void save_config();
    void save_geometry();

    void save_toolbars_visibility(Properties&) const;
    static bool is_toolbar_visible(ToolBar*);

    void remove_mem_view(MemoryView*);

    void update_mem_view(RefPtr<MemoryView>);
    void update_on_main_thread(RefPtr<MemoryView>);

    void print_instance_counted();

    void set_view_mode(ViewType);

    void add_to_breaklist(pid_t tid, Symbol*, std::vector<addr_t>&);

    void schedule_deletion(Gtk::Widget*);

    bool can_interact(); // should be const; but gtk--1.2 does not like it

    void update_stack_view();

    void on_toolbar_font_set();

    bool is_at_debug_event() const;

private:
    typedef std::map<DebuggerCommand*, Gtk::Widget*> CommandMap;

    static const MenuEntry* fileMenu_[];
    static const MenuEntry* openMenu_[];
    static const MenuEntry* editMenu_[];
    static const MenuEntry* viewMenu_[];
    static const MenuEntry* breakpointMenu_[];
    static const MenuEntry* progMenu_[];
    static const MenuEntry* optionsMenu_[];
    static const MenuEntry* toolsMenu_[];
    static const MenuEntry* helpMenu_[];

    Gtk::Label*         statTop_;   // displays current file, etc.
    Gtk::Entry*         statText_;
    Gtk::Entry*         funcBar_;
    StackViewPtr        stackView_; // displays current stack trace
    ThreadViewPtr       threadView_;
    ProgramViewPtr      progView_;
    VariablesViewPtr    localVarView_;
    WatchViewPtr        watches_;

    mutable Mutex       evalMutex_;
    ExprEvalDialog*     evalView_;  // for expression evaluation
    RegisterViewPtr     registerView_;
    ProgressBoxPtr      progressBox_;
    Gtk::Box*           toolbox_;
    ProgramToolBar2*    toolbar_;
    ToolMap             toolMap_;
    Gtk::Menu*          toolMenu_;

    size_t              breakpointCount_;
    bool                waiting_;   // waiting responses from UI
    bool                ownsUserInteraction_;
    atomic_t            atDebugEvent_;
    bool                maximized_;

    MemoryViews         memoryViews_;
    std::string         sourcesPath_;
    MenuList*           viewMode_;

    LayoutManager       layoutMgr_;
    LayoutStrategyPtr   layoutStrategy_;
    std::string         keyBindings_;
    std::auto_ptr<MemoryRequestHandler> memReqHandler_;

    CommandMap          commandMap_;

    std::vector<InterpBoxPtr> interp_;
    std::string         targetParam_;  // do_attach
};


#endif // MAIN_WINDOW_H__9CEB24F3_F2D9_4E9C_9353_C08697880309
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
