#ifndef ZERO_H__82331446_EC88_46A0_9C85_8EEDD168EA79
#define ZERO_H__82331446_EC88_46A0_9C85_8EEDD168EA79
//
// $Id: zero.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#define ZERO_API_MAJOR  1
#define ZERO_API_MINOR 21

#define ZDK_MAJOR ZERO_API_MAJOR
#define ZDK_MINOR ZERO_API_MINOR

#include <sys/types.h>
#include "zdk/assert.h"
#include "zdk/debug_reg.h"
#include "zdk/disasm.h"
#include "zdk/generic_plugin.h"
#include "zdk/memio.h"
#include "zdk/misc.h"
#include "zdk/module.h"
#include "zdk/process.h"
#include "zdk/properties.h"
#include "zdk/register.h"
#include "zdk/runnable.h"
#include "zdk/sym.h"
#include "zdk/stack.h"


struct Target; // opaque

struct BreakPoint;
struct BreakPointAction;
struct Debugger;
struct DebuggerCommand;
struct DebuggerPlugin;
struct DebugSymbol;          // full definition in zdk/debug_sym.h
struct DebugSymbolEvents;    // zdk/debug_sym.h
struct ExprEvents;           // zdk/expr.h
struct ObjectFactory;        // zdk/stream.h
struct Process;
struct TranslationUnit;      // zdk/unit.h
struct TypeSystem;           // zdk/type_system.h

enum
{
    DEFAULT_THREAD = 0,
};



/**
 * Interface for manipulating threads in the debugged program.
 * May refer to a thread that is currently running, or to a
 * crashed thread which has been "reconstructed" from the
 * information in a core file.
 * @see Process, Runnable
 */
DECLARE_ZDK_INTERFACE_(Thread, MemoryIO)
{
    DECLARE_UUID("ec06d99e-c9de-4f20-9b0a-e1a823ffdaf9")

    /**
     * @return opaque handle to the Debug Target that
     * this thread belongs to. Target can be live local
     * process, remote process, core dump file, etc.
     */
    virtual Target* target() const = 0;

    /**
     * @return the Debugger attached to this thread
     */
    virtual Debugger* debugger() const = 0;

    /**
     * @return the thread id (as returned by pthread_create),
     * or zero if the id is not known.
     */
    virtual unsigned long thread_id() const = 0;

    /**
     * @return the the executable's filename
     */
    virtual const char* filename() const = 0;

    /**
     * @return the thread's name -- which may be the
     * same as the filename(), but not necessarily.
     */
    virtual const char* name() const = 0;

    /**
     * @return the light-weight process id of this thread
     */
    virtual pid_t lwpid() const = 0;

    /**
     * @return this thread's group id
     */
    virtual pid_t gid() const = 0;

    /**
     * @return the task id of the parent of this thread
     */
    virtual pid_t ppid() const = 0;

    /**
     * @return the process that this thread is part of
     */
    virtual Process* process() const = 0;

    /**
     * detach from debugger
     */
    virtual void detach() = 0;

    virtual Runnable::State runstate() const = 0;

    /**
     * @return false if loaded from a corefile
     * @todo: move it to Process?
     */
    virtual bool is_live() const = 0;

    /**
     * @return true if this thread is the main thread of
     * a process created with the fork() system call (as
     * opposed to being created with a clone(), or by a
     * pthread_create call).
     */
    virtual bool is_forked() const = 0;

    virtual bool is_execed() const = 0;

    virtual bool is_done_stepping() const = 0;

    virtual bool is_exiting() const = 0;

    /**
     * useful when debugging 32-bit apps on 64-bit systems
     */
    virtual bool is_32_bit() const = 0;

    /**
     * @return true if stepping thru a range of machine
     * instructions that corespond to a line of
     * high level source code
     */
    virtual bool is_line_stepping() const = 0;

    /**
     * @return true if an event on this thread has
     * been queued (because there where several events
     * that occurred at about the same time on
     * different threads).
     */
    virtual bool is_event_pending() const = 0;

    /**
     * @return true if this thread has been
     * intentionally stopped by the debugger.
     *
     * @note do not read too much into this, it is mainly
     * used internally by the engine for managing events
     */
    virtual bool is_stopped_by_debugger() const = 0;

    /**
     * @return true if this thread entered a system
     * call at the specified PC.
     * @param pc program counter
     */
    virtual bool is_syscall_pending(addr_t pc) const = 0;

    virtual void cancel_syscall() = 0;

    // methods for retrieving special registers
    virtual addr_t program_count() const = 0;
    virtual addr_t frame_pointer() const = 0;
    virtual addr_t stack_pointer() const = 0;

    virtual addr_t stack_start() const = 0;

    /**
     * @return value stored in the N-th general purpose register
     * @note it only works for general-purpose registers;
     * If "true" is passed as the second parameter, then the
     * register will be read from the state saved in the active frame
     */
    virtual reg_t read_register(int n, bool useFrame) const = 0;

    /**
     * Returns the status, last obtained by waitpid()
     * when the thread stopped or exited.
     */
    virtual int status() const = 0;

    /**
     * @return last signal received by this thread
     */
    virtual int signal() const = 0;

    virtual void set_signal(int) = 0;

    /**
     * return the thread's map of symbol tables
     */
    virtual SymbolMap* symbols() const = 0;

    virtual bool single_step_mode() const = 0;

    /**
     * Get a stack trace for this thread, containing
     * the specified number of frames.
     */
    virtual StackTrace* stack_trace(size_t = INT_MAX) const = 0;

    /**
     * @return the size of the stack trace
     */
    virtual size_t stack_trace_depth() const = 0;

    /**
     * result of function that returned most recently, may be NULL
     */
    virtual DebugSymbol* func_return_value() = 0;

    virtual long syscall_num() const = 0;

    /**
     * Get the result of the most recent function call in a
     * platform-independent fashion. On an Intel-based system,
     * calling this method returns the content of the EAX register
     * (RAX on a 64-bit x86-64 AMD-based system).
     *
     * @see Runnable::set_result
     */
    virtual word_t result() const = 0;

    /**
     * Similar to result(), on a 32-bit Intel-based machine
     * returns the long long stored in EDX-EAX.
     * @see Runnable::set_result64
     */
    virtual int64_t result64() const = 0;

    /**
     * Collect the result of a function call on this thread
     * that returns a double, or a long double
     * @see Runnable::set_result_double
     */
    virtual long double result_double(size_t size) const = 0;

    /**
     * Associate a value with this thread.
     * @param key a user-supplied key name
     * @param val a user-supplied value
     * @param replace if another value of the same
     * key is found, replace it if this param is true
     */
    virtual void set_user_data(
        const char* key,
        word_t      val,
        bool        replace = true) = 0;

    /**
     * Retrieve user data by key.
     * @return true if found, false if no such key.
     */
    virtual bool get_user_data(const char* key, word_t*) const = 0;

    /**
     * Associate a reference-counted object with this thread.
     * @note the set_user_data et all functions allow for misc.
     * experiments and hacks, either inside the engine proper
     * or in plug-ins;
     *
     * @note By convention, the names of objects and user data
     * used by the engine, start with a . (dot)
     * Integer user data and user objects live in separate
     * "namespaces".
     */
    virtual void set_user_object(
        const char* keyName,
        ZObject*    object,
        bool        replace = true) = 0;

    /**
     * Get user object by key.
     * @return pointer to ref-counted object if found, or
     * NULL if no such key.
     */
    virtual ZObject* get_user_object(const char*) const = 0;

    /**
     * Enumerate the general purpose registers, return number of
     * registers. If a non-null pointer to a callback object is given,
     * then call its notify() method for each register.
     */
    virtual size_t enum_user_regs(EnumCallback<Register*>*) = 0;

    /**
     * Enumerates the general purpose registers, and the FPU and XMM
     * registers, in the same manner as enum_user_regs()
     */
    virtual size_t enum_cpu_regs(EnumCallback<Register*>*) = 0;

    /**
     * @return an object holding the general CPU register contents;
     * the actual object definition is opaque and designed to be
     * used internally by the implementation
     */
    virtual ZObject* regs(ZObject* reserved = NULL) const = 0;

    /**
     * @return an object holding the floation-point register contents;
     * the actual object definition is opaque and designed to be
     * used internally by the implementation
     */
    virtual ZObject* fpu_regs(ZObject* reserved = NULL) const = 0;

    /**
     * @return a hardware-specific interface that provides access
     * to the debug registers.
     */
    virtual DebugRegs* debug_regs() = 0;

    virtual ZObject* action_context() const = 0;

    /**
     * @return the pid of the process that last sent a signal
     * to this thread
     */
    virtual pid_t get_signal_sender() const = 0;

    /**
     *get the return address of the current function
     */
    virtual addr_t ret_addr() = 0;

    /**
     * Normally the execution of the target program is paused
     * whenever a debug event occurs. However, the user may
     * decide that some threads are not of interest.
     *
     * By calling set_traceable with a value of false,
     * the debugger is instructed to ignore further events on
     * this thread.
     */
    virtual void set_traceable(bool) = 0;

    virtual bool is_traceable() const = 0;

    virtual bool exited(int* status = NULL) const = 0;
};



enum EventType
{
    // no event, just prompting the user for a command
    E_PROMPT,

    // a debugged thread stopped because it has received a signal
    E_THREAD_STOPPED,

    // the thread stopped in the debugger with a
    //   SIGTRAP as the result of hitting a breakpoint
    E_THREAD_BREAK,

    // the thread finished running
    E_THREAD_FINISHED,

    E_SYSCALL,

    E_SINGLE_STEP,

    // used internally by the expression evaluator
    //   to indicate that a function call or some other
    //   asynchronous evaluation has completed.
    E_EVAL_COMPLETE,

    // returned from function call
    E_THREAD_RETURN,

    // thread is about to exit
    E_THREAD_EXITING,

    // force to sizeof(uint)
    E_NONE = UINT_MAX,
    E_DEFAULT = E_NONE
};


struct InputStreamEvents;
struct SignalPolicy;


/**
 * Interface to the debugger engine
 */
DECLARE_ZDK_INTERFACE_(Debugger, Unknown2)
{
    DECLARE_UUID("282d4147-5c83-4d01-8eca-4287a9b71fb0")

    /**
     * @return verbosity level -- when higher than zero, the engine
     * prints miscellaneous information to the standard error output.
     */
    virtual int verbose() const = 0;

    /**
     * For debugging object leaks inside the debugger;
     * it's a no-op in the release build.
     */
    virtual void print_counted_objects(const char* func) const = 0;

    // Modes for the Debugger::step method -- see below
    enum StepMode
    {
        STEP_NONE = UINT_MAX,

        STEP_INSTRUCTION = 0,   // execute one CPU instruction
        STEP_OVER_INSTRUCTION,  // ditto, step over routine calls
        STEP_SOURCE_LINE,       // execute one source line
        STEP_OVER_SOURCE_LINE,  // ditto, don't dive into functions
        STEP_RETURN,            // step until current function returns
    };

    /**
     * Bits for set_option, get_option
     */
    enum Option
    {
        OPT_NONE =                  0,
        OPT_HARDWARE_BREAKPOINTS =  1,
        OPT_START_AT_MAIN =         2,
        OPT_TRACE_FORK =            4,
        OPT_BREAK_ON_SYSCALLS =     8,
        OPT_TRACE_SYSCALLS =        16,
        OPT_NO_BANNER =             32,
        OPT_SILENT =                64, // no event info
        OPT_BREAK_ON_THROW =       128,
        OPT_ACCEPT_TARGET_PARAM =  256,
        OPT_SPAWN_ON_FORK =        512, // start new debugger instance
                                        // and attach to forked process
    };

    /**
     * Enumerates all running tasks (in the system)
     * that the debugger can potentially attach to.
     *
     * @param target optional, target-specific description
     */
    virtual size_t enum_user_tasks(EnumCallback<const Runnable*>*,
                                   const char* target = NULL) = 0;

    /**
     * Attach debugger to process
     * @param target optional, target specific description
     */
    virtual void attach(pid_t processID, const char* target = NULL) = 0;

    /**
     * Detach from all attached threads.
     * @note if the program was exec-ed by the debugger,
     * this method will kill all debuggee threads.
     */
    virtual void detach() = 0;

    /**
     * Tokenizes (splits into words) the given command, and then
     * executes it.
     *
     * The user can also specify the environment. Alternatively,
     * the environment can be altered by calling set_environment.
     * @param command
     * @param shellExpandArgs if true, the command is expanded
     * according to shell rules before being tokenized
     * @param env optional environment; if not specified, the
     *  current environment of the debugger process is inherited.
     *
     * @return the process id of the new process
     */
    virtual pid_t exec(const char* command,
                       bool shellExpandArgs = false,
                       const char* const* env = 0) = 0;

    /**
     * Load a core file.
     * @param core the name of the core file
     * @param prog the [optional] name of the program that produced
     * the core dump.
     */
    virtual void load_core(const char* core, const char* prog) = 0;

    /**
     * Retrieves the execution environment for debugged programs.
     */
    virtual const char* const* environment(bool reset = false) = 0;

    /**
     * Sets the environment for the processes to be debugged.
     * The method does not alter the runtime environment of the
     * debugger itself. Rather, it alters a collection of environment
     * variables that are maintained internally by the debugger.
     * The environment, as set by this method, can
     * be overriden by the third argument of the exec method.
     * @note the environment of already running processes that the
     * debugger attaches to is NEVER modified. Calling this method
     * affects subsequent calls of the exec method only.
     */
    virtual void set_environment(const char* const*) = 0;

    /**
     * Instructs the debugger whether to resume execution of
     * debuggee as soon as the processing of the current event
     * completes. Typically, upon some signal is raised inside
     * the debuggee, the debugger enters an interactive prompt
     * mode, asking the user for commands; this methods forces
     * the debugger to exit the prompt (if parameter is true)
     * or keep in interactive mode (if param is false).
     */
    virtual void resume(bool = true) = 0;

    virtual bool is_resumed() const = 0;

    /**
     * Instruct thread to execute in single step mode within
     * the given range of addresses; executions breaks in
     * debugger when the instruction pointer gets out of the
     * specified range.
     */
    virtual void step(Thread*, StepMode, addr_t from, addr_t to) = 0;

    /**
     * Stops execution of all threads in the debugged program.
     */
    virtual void stop() = 0;

    virtual void quit() = 0;

    /**
     * Retrieve thread by lwpid, returns NULL if not found.
     * If the given pid is zero, return the first thread that
     * the debugger is attached to (first in the internal list).
     */
    virtual Thread* get_thread(pid_t, unsigned long = 0) const = 0;

    virtual void set_current_thread(Thread*) = 0;

    virtual Thread* current_thread() const = 0;

    virtual size_t enum_processes(EnumCallback<Process*>* = 0) const = 0;

    /**
     * Enumerate all threads, in all processes.
     */
    virtual size_t enum_threads(EnumCallback<Thread*>* = 0) = 0;

    /**
     * Disassemble a number of addresses starting
     * at given symbol and call the EnumCallback
     * sink once for every line of disassembled code.
     * @param thread current debugged thread
     * @param start symbol to start at
     * @param howMany sizeof the code to disassemble; if
     * membuf is not NULL, the caller must make sure that
     * the size of the buffer is equal or greater.
     * @param mixWithSource if true, source code is mixed
     * with assembly code in the output (where available).
     * @param membuf if NULL, disassemble from the binary
     * file image, otherwise disassemble from this buffer.
     * @param callback output sink
     * @return number of actually disassembled bytes
     */
    virtual size_t disassemble(
        Thread* thread,
        Symbol* start,
        size_t howMany,
        bool mixWithSource,
        const uint8_t* membuf,
        Disassembler::OutputCallback* callback) = 0;

    /**
     * @return the handling policy for given signal
     */
    virtual SignalPolicy* signal_policy(int sigNum) = 0;

    /**
     * Invoke a debugger command, as if typed at the
     * prompt. The second param specifies the thread to
     * which the command applies (or NULL).
     * @note: if the command is not recognized, a runtime
     * exception will be thrown
     * @return true if the debugged program should be resumed
     * (and the interactive mode should be exited)
     */
    virtual bool command(const char* cmd, Thread* = 0) = 0;

    /**
     * Enumerate all loaded debugger plug-ins, and, if a non-null
     * pointer to a callback object is provided, call the notify()
     * method for each plug-in.
     * @return the number of plug-ins
     */
    virtual size_t enum_plugins(EnumCallback<DebuggerPlugin*>*) = 0;

    /**
     * Enumerate global variables.
     * @param thread the thread of context
     * @param name if not null, enumerate globals that match this
     * name, otherwise enumerate all.
     * @param function the ELF symbol that corresponds to the
     * function in current scope; if NULL, the scope of the
     * currently selected stack frame is assumed.
     * @param events pointer to optional callback function object
     * @param scope specifiers the scope of the enumeration
     * (LOOKUP_LOCAL, LOOKUP_ALL, LOOKUP_MODULE, LOOKUP_UNIT)
     * @param enumFunctions if true, include symbols that correspond
     * to function in the enumeration
     * @return the number of enumerated symbols.
     */
    virtual size_t enum_globals(
        Thread* thread,
        const char* name,
        Symbol* function,
        DebugSymbolEvents* events,
        LookupScope scope,
        bool enumFunctions = false) = 0;

    /**
     * Enumerate variables that are visible in the scope of a
     * given function.
     * @param thread the thread of context
     * @param name if not null, enumerate globals that match this
     * name, otherwise enumerate all.
     * @param function the ELF symbol that corresponds to the
     * function in current scope; if NULL, the scope of the
     * currently selected stack frame is assumed.
     * @param events pointer to optional callback function object
     * @param scope specifiers the scope of the enumeration
     * (LOOKUP_LOCAL, LOOKUP_ALL, LOOKUP_MODULE, LOOKUP_UNIT,
     * LOOKUP_PARAM);
     * @param enumFunctions if true, include symbols that correspond
     * to function in the enumeration
     * @note if the scope of the enumeration is other than
     * LOOKUP_LOCAL, then this function will call enum_globals()
     * @return the number of enumerated symbols.
     */
    virtual size_t enum_variables(
        Thread* thread,
        const char* name,
        Symbol* function,
        DebugSymbolEvents* events,
        LookupScope scope = LOOKUP_LOCAL,
        bool enumFunctions = false) = 0;

    /**
     * Evaluate a C-like expression, in the context of given
     * thread and address. Multiple statements, flow control
     * statements (if/else for, etc) are not recognized.
     * C++ fully-qualified names and C++ casts are okay.
     * @return true if expression evaluation is complete,
     * false if more steps are pending.
     */
    virtual bool evaluate(
        const char* expr,
        Thread*     thread,
        addr_t      addr,
        ExprEvents* events,
        int         numericBase = 0) = 0;

    /**
     * This method is to be called by a plugin operation that
     * may take a longer time to complete. If the debugger
     * returns false, the plug-in should cancel the operation.
     * @param what a string describing the current status
     * @param percent percentage of completions
     * @param cookie a value to identify the caller.
     */
    virtual bool progress(
        const char* what,
        double percent,
        word_t cookie = 0) = 0;

    /**
     * @return a pointer to the BreakPointManager interface;
     * @note may return NULL if no program is being debugged.
     */
    virtual BreakPointManager* breakpoint_manager(pid_t = 0) = 0;

    /* Breakpoint management: convenience methods on top of
       the BreakPointManager interface. The methods below
       should satisfy most needs; in addition, one can
       interface_cast to the BreakPointManager interface. */

    /**
     * Arrange for the debugge to stop at the given address.
     * @return false if a user-defined breakpoint already
     * exists at given address.
     * If a breakpoint exists, and all its associated actions
     * are disabled, and the enableExisting parameter is true,
     * then the breakpoint actions are enabled.
     * If no thread is specified, the breakpoint applies to
     * all debugged threads.
     * @param thread current thread of execution
     * @param addr code address where to insert breakpoint
     * @param perThread if true, the breakpoint applies to
     * the current thread of execution only, otherwise it applies
     * to all threads (current and future) that belong to the
     * thread's process.
     */
    virtual bool set_user_breakpoint(
                                    Runnable*,
                                    addr_t addr,
                                    bool enableExisting = false,
                                    bool perThread = false) = 0;

    /**
     * Same as set_user_breakpoint, only that the breakpoint
     * is temporary, i.e. it is automatically removed after
     * being hit.
     */
    virtual BreakPoint* set_temp_breakpoint(Runnable*, addr_t) = 0;

    /**
     * Remove all user-defined breakpoint actions for
     * given process, thread and address.
     * that apply to the given thread are inspected.
     * @note Currently, user-defined actions are indentified
     * by the action name "USER"; this might be replaced in the
     * future with a more reliable mechanism.
     * @return the number of actions removed.
     */
    virtual size_t remove_user_breakpoint(pid_t, pid_t, addr_t) = 0;

    /**
     * Debugged program will stop in debugger when the
     * specified memory address is accessed; the WatchType
     * parameter specifies the type of memory access.
     * @return false if watchpoint could not be set (possibly
     * because all hardware debug registers are in use).
     */
    virtual bool set_watchpoint(
        Runnable* thread,
        WatchType type,
        bool global,
        addr_t addr) = 0;

    /**
     * Arrange for debuggee to stop when a condition is met for
     * a given debug symbol, i.e. the value of the symbol satisfies
     * the specified relation with respect to a value (example:
     * symbol becomes GT -- greater than -- value).
     * @note Currently, this operation works for fundamental types
     * only. A logic_error exception will be thrown for aggregate
     * types such as classes and arrays.
     * @return false if watchpoint could not be set (possibly
     * because all hardware debug registers are in use).
     * @param thread the current task
     * @param symbol the debug symbol to be watched
     * @param relation equal, not equal, greather than, etc.
     * @param value the threshold value
     * @note The value is specified as a string, and will be
     * converted internally to the appropriate datatype.
     * @param global if true, watchpoint applies to all threads,
     * otherwise it applies just to the current thread.
     */
    virtual bool break_on_condition(
        Runnable*,
        DebugSymbol*,
        RelType relop,
        SharedString*,
        bool global = true) = 0;

    virtual void remove_breakpoint_action(BreakPointAction*) = 0;

    /**
     * This method allows a debugger component to install a custom
     * action for dealing with a signal received by the debuggee.
     * For uniformity, the breakpoint action interface is used for
     * the handler. The BreakPointAction::execute() method will
     * receive a NULL breakpoint when used with a signal. If execute()
     * returns true, the handler is kept around, otherwise it is
     * removed after executing.
     * @note The engine does not chain the actions -- this should be
     * done explicitly by calling get_sig_action() and saving previous
     * handlers.
     */
    virtual void set_sig_action(size_t signr, BreakPointAction*) = 0;

    virtual BreakPointAction* get_sig_action(size_t signr) const = 0;

    /**
     * Enumerates all modules, in all attached processes.
     */
    virtual size_t enum_modules(EnumCallback<Module*>*) const = 0;

    virtual void schedule_interactive_mode(Thread*, EventType) = 0;

    typedef EnumCallback2<SymbolTable*, addr_t> AddrEvents;

    /**
     * Given a source file name string, and a line number pair
     * find all the code addresses that correspond to it.
     * @note The engine delegates to the loaded DebugInfoReader
     * plug-ins to find the correspondence between source code
     * lines and the machine code generated by the compiler.
     */
    virtual size_t line_to_addr(
        SharedString*   sourceFile,
        size_t          lineNumber,
        AddrEvents*     events,
        const Thread*   thread,
        bool*           cancelled = NULL) = 0;

    /**
     * @return a pointer to an implementation of the Properties
     * interface; the Properties interface provides a generic
     * mechanism for storing name-value pairs.
     */
    virtual Properties* properties() = 0;

    virtual void reset_properties() = 0;

    /**
     * @return a bitmask of current options.
     */
    virtual uint64_t options() const = 0;

    /**
     * Set the engine options. Currently, the supported options are
     * @li OPT_HARDWARE_BREAKPOINTS
     * @li OPT_START_AT_MAIN
     * @li OPT_BREAK_ON_SYSCALLS
     */
    virtual void set_options(uint64_t flags) = 0;

    /**
     * Find the translation unit that produced the binary code
     * at the specified address.
     * @note the returned object must be wrapped into a RefPtr
     */
    virtual TranslationUnit* lookup_unit_by_addr(Process*, addr_t) const = 0;

    /**
     * Lookup translation unit by canonical source filename.
     *
     * @note the returned object must be wrapped into a RefPtr
     * @note if the given filename is a header file, the first unit
     * that includes it is returned
     */
    virtual TranslationUnit* lookup_unit_by_name(Process*, const char*) const = 0;

    virtual ZObject* current_action_context() const = 0;
    virtual void push_action_context(ZObject*) = 0;
    virtual void pop_action_context() = 0;

    enum MessageType
    {
        MSG_INFO,
        MSG_STATUS,
        MSG_ERROR,
        MSG_YESNO,
        MSG_HELP,
        MSG_UPDATE
    };

    virtual bool message(const char*,
                         MessageType,
                         Thread* = NULL,
                         bool async = false) = 0;

    virtual size_t read_settings(InputStreamEvents*) = 0;

    virtual void save_properties() = 0;

    /**
     * Add a filename and line number that should always be stepped over.
     * This allows the user to step over inlined library code, such as
     * boost or STL templates.
     * @param file filename
     * @param lineNum if 0, step over all functions in given file,
     *  if -1, step over all function in directory, otherwise step
     *  over functions that match the line exactly.
     */
    virtual void add_step_over(SharedString* file, long lineNum) = 0;

    /**
     * Remove step-over entry. If file is NULL, remove all entries.
     */
    virtual void remove_step_over(SharedString* file, long lineNum) = 0;

    /**
     * Enumerate step-over entries, return count
     */
    virtual size_t enum_step_over(EnumCallback2<SharedString*, long>*) const = 0;

    /**
     * @return true if given filename and line match an existing entry
     */
    virtual bool query_step_over(const SymbolMap*, addr_t progCount) const = 0;

    /**
     * @note experimental
     */
    virtual size_t enum_tables_by_source(
        SharedString*,
        EnumCallback<SymbolTable*>*) const = 0;

    /**
     * For internal use only: enter TargetManager critical section.
     */
    virtual void enter() = 0;
    virtual void leave() = 0;

    virtual void set_breakpoint_at_throw(Thread*, bool permanent = true) = 0;
};



DECLARE_ZDK_INTERFACE_(VersionInfo, struct Unknown)
{
    DECLARE_UUID("c8b602e3-ec01-4936-8413-ec379932e92b")

    /**
     * Returns major version, and fills out minor and revision
     * if they are not NULL.
     */
    virtual uint32_t version(
        uint32_t* minor     = NULL,
        uint32_t* revision  = NULL) const = 0;

    virtual const char* description() const = 0;
    virtual const char* copyright() const = 0;
};



/**
 * Debugger component interface
 */
DECLARE_ZDK_INTERFACE_(DebuggerPlugin, GenericPlugin)
{
    DECLARE_UUID("e925836a-3f62-4fcc-b776-b9eb767bccce")

    /**
     * Initialize plugin with pointer to debugger and cmd line.
     */
    virtual bool initialize(Debugger*, int* argc, char*** argv) = 0;

    /**
     * Some plug-ins may require an explicit start
     */
    virtual void start() = 0;

    /**
     * indicates to the plugin that it is about to be
     * unloaded, and it should gracefully release resources
     */
    virtual void shutdown() = 0;

    /**
     * The debugger engine calls this method to give plugins the
     * opportunity to register any streamable objects (objects that
     * can be read from a saved state stream, for example).
     * @note this methods is called before initialize()
     */
    virtual void register_streamable_objects(ObjectFactory*) = 0;

    /**
     * This method is called whenever the debugger
     * is about to read a new symbol table.
     */
    virtual void on_table_init(SymbolTable*) = 0;

    /**
     * Called when the debugger has finished reading
     * a symbol table.
     */
    virtual void on_table_done(SymbolTable*) = 0;

    /**
     * Called when the debugger attaches itself to a new thread
     */
    virtual void on_attach(Thread*) = 0;

    /**
     * Called for each thread that finishes, and with a
     * NULL thread when the debugger detaches from the
     * program.
     */
    virtual void on_detach(Thread* = 0) = 0;

    /**
     * This method is called by the engine to notify the
     * plugin that a thread has entered, or is about to
     * leave a system call; the plugin is responsible for
     * pairing up the enter/leave events.
     * @param sysCallNum the system call number.
     * @note The engine does not emit this notification
     * by default, you need to call Debugger::set_option
     * with OPT_BREAK_ON_SYSCALLS or OPT_BREAK_TRACE_SYSCALLS.
     * @see Debugger::set_option
     */
    virtual void on_syscall(Thread*, int sysCallNum) = 0;

    /**
     * Notification sent to plug-ins when the debuggee stops.
     * The plug-in is given the opportunity to take over
     * handling the event. If this method returns true, then
     * the notification is not passed around to other
     * plug-ins, and the internal handling is skipped (i.e.
     * the text-mode command prompt is not shown).
     * This way, a plug-in that implements a GUI or a
     * custom command-line, can take control the interaction
     * with the user.
     * @note the on_event notification is sent went the engine
     * is about to show the command prompt.
     */
    virtual bool on_event(Thread*, EventType) = 0;

    /**
     * Plug-in is notified that the debugger has resumed
     * all threads in the debugged program.
     */
    virtual void on_program_resumed() = 0;

    /**
     * The engine sends this message to all loaded plug-ins when
     * a new breakpoint is inserted, or when the actions associated
     * with an existing breakpoint have been re-enabled.
     * @note: the plugin should make no assumption about the timing
     * of this notification, i.e. the engine may send the message
     * either before or after the actual removal.
     */
    virtual void on_insert_breakpoint(volatile BreakPoint*) = 0;

    /**
     * The engine sends this message to all loaded plug-ins when
     * a breakpoint is removed.
     * @note: the plugin should make no assumption about the timing
     * of this notification, i.e. the engine may send the message
     * either before or after the actual removal.
     */
    virtual void on_remove_breakpoint(volatile BreakPoint*) = 0;

    /**
     * When a plug-in calls Debugger::progress, the
     * notification is being passed to all the loaded
     * plug-ins. The initiator may choose to ignore the
     * event if it identifies the cookie as being the
     * same value as passed to the progress() call.
     */
    virtual bool on_progress(
        const char* what,
        double percent,
        word_t cookie = 0) = 0;

    virtual bool on_message(const char*,
                            Debugger::MessageType,
                            Thread*,
                            bool async) = 0;
};



/**
 * This interface can be implemented by plugins that are
 * interested in monitoring the loading and unloading of
 * ELF symbol information.
 * @note currently not implemented.
 */
DECLARE_ZDK_INTERFACE_(SymbolMonitor, Unknown2)
{
    DECLARE_UUID("65cbd69c-352f-4c82-9a30-7e00671b50de")
    /**
     * Called upon reading a symbol table. Plug-ins get the
     * opportunity to reject symbols (by returning false) or
     * to memorize them.
     * @note may cause performance issues
     * @note plugins may break the engine by rejecting
     *  symbols of interest (such as __nptl_thread_events)
     */
    virtual bool on_symbol(SymbolTable*, Symbol*) = 0;

    /**
     * Called when the debugger detects that new symbols
     * have been mapped into memory from a dynamic library.
     * @param thread the thread that loaded the library.
     * @param addr address where mapped in memory
     * @param len length in bytes of the mapped area
     * @param symtab pointer to the first symbol table of the
     * loaded file -- use SymbolTable::next to iterate over
     * tables.
     */
    virtual void on_map(
        Thread* thread,
        addr_t  addr,
        size_t  len,
        SymbolTable*) = 0;

    /**
     * Called when a shared library is unloaded
     */
    virtual void on_unmap(
        Thread* thread,
        addr_t addr,
        size_t len,
        SymbolTable*) = 0;
};



/**
 * @return a NON-NULL Runnable interface
 * @throw logic_error if the passed thread object
 * does not implement the Runnable interface.
 */
Runnable* get_runnable(Thread* thread);
Runnable* get_runnable(Thread* thread, std::nothrow_t);


// Copyright (c) 2004, 2005, 2006, 2007 Cristian L. Vlasceanu

#endif // ZERO_H__82331446_EC88_46A0_9C85_8EEDD168EA79
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
