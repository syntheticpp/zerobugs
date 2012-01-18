#ifndef DEBUGGER_SHELL_H__1E4882EA_7E92_451D_A2D3_1BEB566FCE89
#define DEBUGGER_SHELL_H__1E4882EA_7E92_451D_A2D3_1BEB566FCE89
//
// $Id: debugger_shell.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <set>
#include <string>
#include <vector>
#include "dharma/redirect.h"
#include "debugger_engine.h"

class Command;
class Backtrace;
class SourceListing;

typedef std::auto_ptr<SourceListing> ListingPtr;


/**
 * Command-line user interface on top of the DebuggerEngine.
 * Provides simple user interaction with the debugger.
 */
CLASS DebuggerShell : public DebuggerEngine, Disassembler::OutputCallback
{
public:
    // non-copyable, non-assignable
    DebuggerShell(const DebuggerShell&);
    DebuggerShell& operator=(const DebuggerShell&);

    DebuggerShell();

    ~DebuggerShell() throw();

    static DebuggerShell& instance();

    static void auto_complete_impl(
        const char*,
        const std::string&,
        std::vector<std::string>&);

    static void handle_signal(int sig, siginfo_t*, void*);

    void print_banner() const;

    void print_debug_symbol(DebugSymbol*, size_t, size_t, int, bool = true);

    void release() { delete this; }

protected:
    virtual void on_idle();
    virtual void on_event(Thread&);

    virtual bool on_interface(DynamicLibPtr, uuidref_t, Unknown2*&);

    void begin_interactive_mode(Thread*, EventType, Symbol*);

    bool command(const char*, Thread*);

    /**
     * Prompt the user for a command; if a plugin is interested
     * in handling the event, then the engine doesn't show the prompt,
     * assuming that the plugin will prompt the user instead.
     * This mechanism  allows plugins to implement custom readline-like
     * interfaces, or graphical user interfaces.
     * @note the first plugin that responds TRUE to
     * DebuggerEngine::publish_event() grabs exclusive control over
     * the interaction with the user (i.e. there can be only one active
     * prompt at a given time).
     */
    void prompt_user(RefPtr<Thread>, EventType = E_NONE);

    /**
     * Helper invoked from prompt_user() -- shows a snippet of listing.
     */
    void show_listing(const RefPtr<Thread>&);

    void resume(bool flag);
    bool is_resumed() const;

    void set_current_thread(Thread*);

    Thread* current_thread() const { return current_.get(); }

    void add_command(DebuggerCommand*);

private:
    void print_return_value(const RefPtr<Thread>&);

    /**
     * Dispatch a command to one of the a cmd_... methods
     */
    bool dispatch_command(Thread*, const std::string&);

    DebuggerCommand* lookup_command(const std::string&);

    static char* auto_complete(const char*, size_t);

    static void auto_complete_command(
        const char*, std::vector<std::string>&);

    virtual void print_help(std::ostream&) const;

    /**
     * handle the signal that was caught most recently by
     * the static handle_signal function
     */
    virtual void handle_signal_impl(int sig);

    /**
     * disassembler output callback
     */
    bool notify(addr_t, const char*, size_t);
    bool tabstops(size_t*, size_t*) const;

    /*** interactive commands ***/
    static Command cmd_[];

    bool cmd_addmod(Thread*, const std::vector<std::string>&);
    bool cmd_attach(Thread*, const std::vector<std::string>&);
    bool cmd_break(Thread*, const std::vector<std::string>&);
    bool cmd_clear(Thread*, const std::vector<std::string>&);

    /**
     * Print the current number of instances for various
     * objects -- for debugging leaks in the debugger
     */
    bool cmd_count_objects(Thread*, const std::vector<std::string>&);

    bool cmd_continue(Thread*, const std::vector<std::string>&);
    bool cmd_detach(Thread*, const std::vector<std::string>&);
    bool cmd_disassemble(Thread*, const std::vector<std::string>&);
    bool cmd_dump(Thread*, const std::vector<std::string>&);
    bool cmd_eval(Thread*, const std::vector<std::string>&);
    bool cmd_find(Thread*, const std::vector<std::string>&);
    bool cmd_enable(Thread*, const std::vector<std::string>&);
    bool cmd_exec(Thread*, const std::vector<std::string>&);
    bool cmd_frame(Thread*, const std::vector<std::string>&);
    bool cmd_freeze(Thread*, const std::vector<std::string>&);
    bool cmd_help(Thread*, const std::vector<std::string>&);
    bool cmd_handle(Thread*, const std::vector<std::string>&);
    bool cmd_loadcore(Thread*, const std::vector<std::string>&);
    bool cmd_line(Thread*, const std::vector<std::string>&);
    bool cmd_list(Thread*, const std::vector<std::string>&);
    bool cmd_lookup(Thread*, const std::vector<std::string>&);
    bool cmd_navigate_stack(Thread*, const std::vector<std::string>&);
    bool cmd_open(Thread*, const std::vector<std::string>&);
    bool cmd_print(Thread*, const std::vector<std::string>&);
    bool cmd_reg(Thread*, const std::vector<std::string>&);
    bool cmd_next(Thread*, const std::vector<std::string>&);
    bool cmd_quit(Thread*, const std::vector<std::string>&);
    bool cmd_profile(Thread*, const std::vector<std::string>&);
    bool cmd_restart(Thread*, const std::vector<std::string>&);
    bool cmd_return(Thread*, const std::vector<std::string>&);
    bool cmd_set_next(Thread*, const std::vector<std::string>&);
    bool cmd_step(Thread*, const std::vector<std::string>&);
    bool cmd_show(Thread*, const std::vector<std::string>&);
    bool cmd_symtab(Thread*, const std::vector<std::string>&);
    bool cmd_switch_thread(Thread*, const std::vector<std::string>&);
    bool cmd_watch(Thread*, const std::vector<std::string>&);
    bool cmd_where(Thread*, const std::vector<std::string>&);

    // for testing / deubgging -- yield control to debuggee
    // for specified amount of seconds
    bool cmd_yield(Thread*, const std::vector<std::string>&);

    void dump(std::ostream&, Thread&, addr_t begin, addr_t end);

    void select_frame(Thread&, int);

private:
    static DebuggerShell*   theDebugger_;
    CommandList             commands_;
    addr_t                  disasmAddr_;

    // break out of the prompt_user loop when true
    bool                    resume_;

    RefPtr<Thread>          current_;
    bool                    promptLoopActive_;
    ListingPtr              listing_;
    size_t                  disasmLineCount_;
    std::auto_ptr<Redirect> outputRedirect_;
};


// Copyright (c) 2004, 2006 Cristian L. Vlasceanu

#endif // DEBUGGER_SHELL_H__1E4882EA_7E92_451D_A2D3_1BEB566FCE89

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
