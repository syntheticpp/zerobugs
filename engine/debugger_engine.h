#ifndef DEBUGGER_ENGINE_H__0AEF18F9_E867_46BB_82FA_D62AD7682383
#define DEBUGGER_ENGINE_H__0AEF18F9_E867_46BB_82FA_D62AD7682383

#include "zdk/zero.h"
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
#include "zdk/command.h"
#include "zdk/check_ptr.h"
#include "zdk/data_filter.h"
#include "zdk/frame_handler.h"
#include "zdk/observer_impl.h"
#include "zdk/update.h"

#include <deque>
#include <iosfwd>
#include <stack>
#include <boost/utility.hpp>
#include "breakpoint.h"
#include "breakpoint_mgr.h"
#include "debugger_base.h"
#include "dharma/exec_arg.h"
#include "dharma/plugin_manager.h"
#include "dharma/symbol_util.h"


struct Disassembler;
struct ExprObserver;

class BreakPointManagerImpl;
class RunnableImpl;
class StepOverManager;
class SymbolTableEvents;
class TypeSystemImpl;


/**
 * The DebuggerEngine adds breakpoint and plug-in management
 * on top of the base debugger class. It also implements a
 * mechanism for detecting the creation of new threads;
 * detects the dynamic loading and unloading of shared objects,
 * so that symbol tables can be updated.
 */
class DebuggerEngine : public DebuggerBase
                     , public CommandCenter
                     , public PluginManager
                     , public VersionInfo
                     , private boost::noncopyable
{
protected:
    typedef std::deque<RefPtr<BreakPoint> > BreakPointList;
    typedef std::vector<RefPtr<DebuggerCommand> > CommandList;


    DebuggerEngine();
    virtual ~DebuggerEngine() throw();

    struct EventSchedule
    {
        explicit EventSchedule(EventSchedule*&);
        ~EventSchedule();

        EventSchedule*& self_;

        BreakPointList pending_;
        BreakPointList pendingEnable_;
        EventType eventType_;
        bool interactive_;
        bool step_;
        bool exprEventsHandled_;

    private:
        EventSchedule(const EventSchedule&);
        EventSchedule& operator=(const EventSchedule&);
    }; // EventSchedule

    void critical_error(Thread*, const char*);

    // CommandCenter interface
    void add_command(DebuggerCommand*);
    void enable_command(DebuggerCommand*, bool);

    void shutdown_plugins();

public:
    typedef std::deque<ImportPtr<DebuggerPlugin> > PluginList;
    typedef std::vector<ImportPtr<FrameHandler> > FrameHandlers;
    typedef std::vector<ImportPtr<GenericPlugin> > GenericPlugins;

    DECLARE_UUID("fff97374-3149-4dac-89ad-9884a1572acf")

    BEGIN_INTERFACE_MAP(DebuggerEngine)
        INTERFACE_ENTRY(DebuggerEngine)
        INTERFACE_ENTRY(CommandCenter)
        INTERFACE_ENTRY(VersionInfo)
        INTERFACE_ENTRY_AGGREGATE(breakPointMgr_.get())
        INTERFACE_ENTRY_AGGREGATE(disasm_.get())
        INTERFACE_ENTRY_AGGREGATE(dataFilter_.get())
        INTERFACE_ENTRY_AGGREGATE(updateMgr_)
        INTERFACE_ENTRY_INHERIT(DebuggerBase)
    END_INTERFACE_MAP()

    static const int ENGINE_MAJOR = ZERO_API_MAJOR;
    static const int ENGINE_MINOR = ZERO_API_MINOR;

    static const int ENGINE_REVISION = 140;

    /**
     * Parse the command line arguments.
     * Arguments that do not start with a dash, and args that
     * follow a standalone double dash are pushed to the ExecArg;
     * memorize argc (up to a double-dash, if any) and argv, so
     * that they can be passed down to plug-ins.
     * @return true if the debugger should continue executing,
     * false otherwise.
     */
    bool initialize(int argc, char* argv[], ExecArg&);

    /**
     * Perform cleanup, shut down plug-ins gracefully.
     * Called by quit() and by dtor.
     */
    void shutdown();

    virtual void print_help(std::ostream&) const { };

    template<typename F> F for_each_plugin(F pred)
    {
        // the list may change as we iterate over it
        PluginList tmp(plugins_);

        PluginList::iterator i = tmp.begin(), end = tmp.end();
        for (; i != end; ++i)
        {
            ActionScope scope(actionContextStack_, *i);
            pred(*i);
        }
        return pred;
    }

    virtual void detach();

    virtual void quit();

    /**
     * Publish event to all DebuggerPlugin components
     */
    bool publish_event(Thread*, EventType = E_NONE);

    virtual void begin_interactive_mode(Thread*, EventType, Symbol* = 0);

    virtual size_t line_to_addr(
        SharedString*   sourceFile,
        size_t          lineNumber,
        AddrEvents*     events,
        const Thread*   thread,
        bool*           cancelled = NULL);

    virtual bool progress(const char*, double, word_t);

    virtual void step(Thread*, StepMode, addr_t from, addr_t to);

    /**
     * Continue execution up to the next source line,
     * from the given program count (zero means current).
     * If FROM is zero, the current program counter is assumed;
     * if TO is zero, the engine will attempt to automatically
     * determine the address of the next line.
     */
    bool next(Thread*, addr_t from, addr_t to);

    void step(Thread*, addr_t from, bool machineCode);

    virtual void on_attach(Thread&);

    void cleanup(Thread&);

    virtual void on_event(Thread&);

    /**
     * Called when a "memory breakpoint" (aka watchpoint) is hit
     * @note watchpoints are implemented with hardware breakpoints
     */
    void on_watchpoint(Runnable&, Thread&, addr_t, reg_t);

    /**
     * Called when the debuggee gets a SIGTRAP because
     * a thread is being single-stepped.
     */
    void on_single_step_or_syscall(Runnable&, Thread&);

    /**
     * Handle the system call case, called by
     * on_single_step_or_syscall
     */
    void on_syscall(Runnable&, Thread&, addr_t programCount);

    /**
     * Handle SIGTRAP events
     */
    void on_trap(Thread&);

    void on_hardware_break(Runnable&, Thread&, addr_t, reg_t);

    bool on_user_event(Thread*, int& resume);

    void on_resumed();

    virtual bool on_interface(DynamicLibPtr, uuidref_t, Unknown2*&);

    virtual void on_scan_plugins_complete();

    /**
     * @return the symbol table callback object to be used
     * when reading symbol tables.
     */
    virtual SymbolTableEvents* symbol_table_events();

    /**
     * Set a user breakpoint, returns false if there
     * is a breakpoint already set at the given addr.
     */
    bool set_user_breakpoint(Runnable*, addr_t, bool = false, bool = false);

    size_t remove_user_breakpoint(pid_t procID, pid_t lwpThreadID, addr_t);

    void activate_deferred_breakpoint(BreakPoint*, const SymbolTable&);

    /**
     * Remove actions that match the name, thread and address
     */
    size_t remove_breakpoint_action(pid_t, pid_t, addr_t, const char*);

    void remove_breakpoint_action(BreakPoint::Action*);

    /**
     * Set a one-time breakpoint that is automatically
     * removed after being hit.
     */
    BreakPoint* set_temp_breakpoint(Runnable*, addr_t);

    /**
     * Print all breakpoints (user-defined as well as internal) to
     * an output stream, for debug purposes.
     */
    void print_breakpoints(std::ostream&, pid_t = 0) const;

    /**
     * Debugged program will stop in debugger when the
     * specified memory address is accessed; the WatchType
     * parameter specifies the type of memory access.
     * @return false if watchpoint could not be set (possibly
     * because all hardware debug registers are in use).
     */
    virtual bool set_watchpoint(Runnable*, WatchType, bool, addr_t);

    bool break_on_condition(Runnable* thread,
                            DebugSymbol* symbol,
                            RelType relation,
                            SharedString* value,
                            bool global);

    bool remove_watchpoint_action(BreakPoint::Action*);

    /**
     * @return the breakpoint manager interface, which may be NULL
     * when no program is being debugged
     */
    BreakPointManager* breakpoint_manager(pid_t pid = 0);

    template<typename T>
    RefPtr<BreakPoint> get_breakpoint(const T& t, addr_t a, const Thread* owner = 0)
    {
        RefPtr<BreakPoint> bpnt;
        if (breakpoint_manager())
        {
            bpnt = CHKPTR(breakPointMgr_)->get_breakpoint(t, a, owner);
            assert(!bpnt || !bpnt->is_deferred());
        }
        return bpnt;
    }

    size_t disassemble(Thread* thread,
                       Symbol* start,
                       size_t length,
                       bool mixWithSource,
                       const uint8_t* buffer,
                       Disassembler::OutputCallback*);

    BreakPointAction* get_sig_action(size_t signr) const;

    void set_sig_action(size_t signr, BreakPointAction*);

    size_t enum_plugins(EnumCallback<DebuggerPlugin*>*);

    size_t enum_globals(Thread*,
                        const char*,
                        Symbol*,
                        DebugSymbolEvents*,
                        LookupScope,
                        bool);

    size_t enum_variables(Thread* thread,
                        const char* name,
                        Symbol* function,
                        DebugSymbolEvents*,
                        LookupScope,
                        bool);

    bool evaluate(const char*, Thread*, addr_t, ExprEvents*, int = 0);

    //
    // VersionInfo interface
    //
    uint32_t version(uint32_t* = NULL, uint32_t* = NULL) const;
    const char* description() const;
    const char* copyright() const;

    void init_frame_handlers(const Thread&);

    /**
     * Get the next frame in the stack trace, using the registered
     * FrameHandler interfaces.
     * @return NULL if no handler is registered, or the next frame
     * could not be found
     */
    RefPtr<Frame> unwind_frame(const Thread&, const Frame&);

    /**
     * If events are pending, set a flag to to indicate that
     * interactive mode should start as soon as possible,
     * otherwise enter interactive mode immediately
     */
    void schedule_interactive_mode(Thread*, EventType);

    TranslationUnit* lookup_unit_by_addr(Process*, addr_t) const;
    TranslationUnit* lookup_unit_by_name(Process*, const char*) const;

    bool message(const char*, MessageType, Thread*, bool async = false);

    size_t read_settings(InputStreamEvents*);

    /**
     * Add a filename and line number that should always be stepped over.
     * This allows the user to step over inlined library code, such as
     * boost or STL templates.
     * @param file filename
     * @param lineNum if 0, step over all functions in given file,
     *  if -1, step over all function in directory, otherwise step
     *  over functions that match the line exactly.
     */
    virtual void add_step_over(SharedString* file, long lineNum);

    /**
     * Remove step-over entry. If file is NULL, remove all entries.
     */
    virtual void remove_step_over(SharedString* file, long lineNum);

    /**
     * Enumerate step-over entries, return count
     */
    virtual size_t enum_step_over(EnumCallback2<SharedString*, long>*) const;

    /**
     * @return true if given filename and line match an existing entry
     */
    virtual bool query_step_over(const SymbolMap*, addr_t progCount) const;

    void restore_step_over_properties();

    /**
     * Create a breakpoint action that causes the debugger
     * to break into interactive mode (command line prompt, or UI,
     * depending on what plug-ins are installed)
     *
     * If the permanent flag is set to false, the action is temporary,
     * i.e. it is automatically removed after being executed once.
     */
    RefPtr<BreakPointAction> interactive_action(
        const char* name,
        bool permanent = true);

    void set_options(uint64_t);

private:
    //
    // track the context of action performed on the debuggee
    //
    typedef std::stack<RefPtr<ZObject> > ActionContextStack;

    struct ZDK_LOCAL ActionScope : boost::noncopyable
    {
        ActionContextStack& stack_;

        template<typename T>
        ActionScope(ActionContextStack& stack, const T& p)
            : stack_(stack)
        {
            stack_.push(interface_cast<ZObject*>(p));
        }
        ~ActionScope()
        {
            if (stack_.empty())
            {
                throw std::logic_error("empty action stack");
            }
            stack_.pop();
        }
    }; // ActionScope

    void break_into_interactive_mode(RefPtr<Thread>, RefPtr<BreakPoint>);

    void break_at_catch(RefPtr<Thread>, RefPtr<BreakPoint>);
    void break_at_throw(RefPtr<Thread>, RefPtr<BreakPoint>);

    void schedule_actions(Runnable&, Thread&, BreakPoint&);

    void schedule_actions(Runnable&, Thread&, BreakPoint&, addr_t);
    /**
     * schedule actions associated with given Software Breakpoint
     */
    bool schedule_soft(Runnable&, Thread&, BreakPoint*, addr_t);

    void exec_pending_actions(RunnableImpl*, Thread&);

    /**
     * For the given runnable thread, execute the actions associated
     * with a breakpoint.
     */
    void execute_actions(Runnable&, Thread&, BreakPoint&);

    /**
     * Set breakpoints for reporting thread events
     */
    bool set_thread_event_breakpoints(Thread&, ProcessImpl* = NULL);

    /**
     * Set breakpoints for reporting dynamic linker events
     */
    void set_linker_breakpoint(ProcessImpl&);

    // Callbacks for the BreakPointManager
    void on_insert(volatile BreakPoint*);
    void on_remove(volatile BreakPoint*);

    /**
     * Helper for enum_variables, enumerates variables visible
     * in the scope of a given function.
     * @param thread the thread context
     * @param name if not NULL, filter variables by name
     */
    size_t enum_locals( Thread&     thread,
                        Frame&      frame,
                        const char* name,
                        Symbol&     function,
                        DebugSymbolEvents*,
                        bool        paramOnly = false);

    /**
     * Helper for enum_variables, enumerates variables visible
     * in the scope of a given method.
     */
    size_t enum_members(Thread&     thread,
                        Frame&      frame,
                        const char* name,
                        Symbol&     function,
                        DebugSymbolEvents*);

    size_t enum_funcs(Thread&, const char*, DebugSymbolEvents&);

    bool start_at_main(Thread&);

    void set_use_hardware_breakpoints(bool);

    bool use_hardware_breakpoints() const
    {
        return (options() & OPT_HARDWARE_BREAKPOINTS);
    }

    /**
     * check if there's any breakpoint that needs to fire
     * at the given address
     */
    bool try_breakpoints(Runnable&, addr_t);

    void detach_targets();

    virtual ZObject* current_action_context() const;
    virtual void push_action_context(ZObject*);
    virtual void pop_action_context();

    bool next_line_hack(ThreadImpl&, const RefPtr<BreakPoint>&, EventType);

    void set_breakpoint_at_catch(Thread*);
    void set_breakpoint_at_throw(Thread*, bool permanent = true);

    void set_breakpoint(Thread&, const SymbolEnum&, BreakPoint::Action*);

    void check_unknown_cmdline_opts();
    void add_plugin_custom_commands();

    size_t enum_tables_by_source(SharedString*,
                                 EnumCallback<SymbolTable*>*
                                ) const;
private:
    PluginList plugins_; // all plug-ins
    ActionContextStack actionContextStack_;

    ImportPtr<Disassembler> disasm_; // disassembler plug-in
    FrameHandlers frameHandlers_;

    ImportPtr<DataFilter> dataFilter_;

    RefPtr<BreakPointManagerGroup> breakPointMgr_;

    RefPtr<SymbolTableEvents> symbolEvents_;
    std::vector<RefPtr<BreakPointAction> > sigActions_;

    RefPtr<ExprObserver> exprObserver_;
    boost::shared_ptr<StepOverManager> stepOverMgr_;

    // command line args
    int     argc_;
    char**  argv_;

    EventSchedule* sched_;

    CommandList addons_; // by add_command

    GenericPlugins genericPlugins_;
    bool pluginsInitialized_;
    bool quitCommandPending_;
    bool shutdownPending_;

    RefPtr<Updateable> updateMgr_;
};

#endif // DEBUGGER_ENGINE_H__0AEF18F9_E867_46BB_82FA_D62AD7682383
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
