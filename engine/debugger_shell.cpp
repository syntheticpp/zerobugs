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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>              // for FILE, needed by readline
#include <signal.h>
#ifdef HAVE_UNISTD_H
 #include <unistd.h>            // getpid()
#endif
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#ifdef __linux__
 #include <execinfo.h>
#endif
#ifdef USE_GNU_READLINE
 #include <readline/history.h>
 #include <readline/readline.h> // GNU readline
#endif
#include <fstream>              // ofstream
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <boost/tokenizer.hpp>
#include "dharma/canonical_path.h"
#include "dharma/directory.h"
#include "dharma/environ.h"
#include "dharma/exec_arg.h"
#include "dharma/process_name.h"
#include "dharma/sigutil.h"
#include "dharma/symbol_util.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"
#include "generic/auto_file.h"
#include "generic/state_saver.h"
#include "generic/temporary.h"
#include "interp/variant_impl.h"
#include "zdk/breakpoint_util.h"
#include "zdk/check_ptr.h"
#include "zdk/expr.h"
#include "zdk/history.h"
#include "zdk/fheap.h"
#include "zdk/shared_string_impl.h"
#include "zdk/signal_policy.h"
#include "zdk/thread_util.h"
#include "zdk/types.h"
#include "zdk/utility.h"
#include "zdk/variant_util.h"
#include "zdk/zobject_scope.h"
#include "readline/read_line.h"
#include "readline/terminal.h"
#include "debugger_shell.h"
#include "internal_cmd.h"
#include "source.h"
#include "sysid.h"
#include "thread.h"
#include "version.h"

using namespace std;

#if defined(USE_GNU_READLINE) && (RL_READLINE_VERSION < 0x0403)
  #define rl_compentry_func_t Function
#endif

size_t max_array_range();

static const char banner[] = "Zero (v%d.%d.%d " __DATE__ " " __TIME__
")\nThe Linux Application Debugger (%s %s/%s) %s@%s\n%s";

static void auto_complete_breakpoints(const char*, vector<string>&);
static void auto_complete_debug_sym(const char*, vector<string>&);
static void auto_complete_disabled_breakpoints(const char*, vector<string>&);
static void auto_complete_enabled_breakpoints(const char*, vector<string>&);
static void auto_complete_core(const char*, vector<string>&);
static void auto_complete_fname(const char*, vector<string>&);
static void auto_complete_pid(const char*, vector<string>&);
static void auto_complete_sym(const char*, vector<string>&);
static void auto_complete_show(const char*, vector<string>&);
static void auto_complete_sig(const char*, vector<string>&);
static void auto_complete_thread(const char*, vector<string>&);


DebuggerShell* DebuggerShell::theDebugger_ = 0;

Command DebuggerShell::cmd_[] =
{
    { "@", &DebuggerShell::cmd_line, 0,
      "Print current source file name and line number\n"
    },
    { "%r", &DebuggerShell::cmd_reg, 0,
      " <N>: print value of N-th general purpose CPU register\n"
    },
    { "addmod", &DebuggerShell::cmd_addmod, auto_complete_fname,
      " [module-name]: add a module (shared object or executable)\n"
      "to the internally-managed list of modules needed by the "
      "program being debugged\n"
    },
    { "attach", &DebuggerShell::cmd_attach, auto_complete_pid,
      " <pid>: attach debugger to a process. You can type\n"
      "'attach <TAB>' for a list of all the running process ids."
    },
    { "break",  &DebuggerShell::cmd_break, auto_complete_sym,
      " <addr> | <function-name> | <filename>:<line-number>\n"
      "set a breakpoint at given address, function, or source line."
    },
    { "bt", &DebuggerShell::cmd_where, 0,
      " [depth]: print backtrace, optional argument controls the depth of\n"
      "the backtrace (how many steps back to show); same as 'where'"
    },
    { "clear", &DebuggerShell::cmd_clear, auto_complete_breakpoints,
      " [addr...]: clear breakpoint(s) at given addresses."
    },
    { "close", &DebuggerShell::cmd_open, 0, "stop redirecting output."
    },
    { "continue", &DebuggerShell::cmd_continue, 0, ": resume program execution" },
#ifdef DEBUG_OBJECT_LEAKS
    { "count_objects", &DebuggerShell::cmd_count_objects, 0, 0 },
#endif
    { "detach", &DebuggerShell::cmd_detach, 0,
      ": detach from debugged process."
    },
    { "disable", &DebuggerShell::cmd_enable, auto_complete_enabled_breakpoints,
     " [addr [addr]]: Disable a list of breakpoints, specified by their addresses.\n"
    },
    { "disassemble", &DebuggerShell::cmd_disassemble, 0,
      "[at <addr>] [how_many]: Disassembles debugged program starting\n"
      "at given address, in the given thread, or at current instruction pointer\n"
      "if no address specified."
    },
    { "down", &DebuggerShell::cmd_navigate_stack, 0, "move down the stack trace."
    },
    { "dump", &DebuggerShell::cmd_dump, 0,
      "begin_addr [end_addr]: dumps memory contents to console. If end_addr is not\n"
      "specifed, a default chunk of 64 bytes, starting at begin_addr, is dumped. "
      "If end_addr\nis lower than begin_addr, then the range is reversed.\n"
    },
    { "enable", &DebuggerShell::cmd_enable, auto_complete_disabled_breakpoints,
     " [addr [addr]]: Enable a list of breakpoints, specified by their addresses.\n"
    },
    { "exec", &DebuggerShell::cmd_exec, auto_complete_fname,
      "[progname] [arg [arg]]: Start a program and attach to it.\n"
      "If the debugger is currently attached to another program,\n"
      "it automatically detaches before attaching to the new one.\n\n"
       "NOTE: arguments that follow the program name, and do not start with a dash,\n"
       "are passed as command line arguments to the debugged program. If an argument\n"
       "contains spaces, then you need to put double quotes around it.\n"
       "Arguments starting with a dash are passed to the debugger engine and its\n"
       "plug-ins. If you need to pass command line arguments starting with a dash to\n"
       "the debugged program, prefix them with a standalone, double dash.\n"
       "For example: zero foo -v --main passes -v and --main to the debugger engine,\n"
       "while: zero foo -- -v --main passes -v and --main to the debugged program foo.\n"
    },
    { "eval", &DebuggerShell::cmd_eval, auto_complete_debug_sym,
      " [expr [expr]]: evaluate expressions. NOTE: This command\n"
      "can be used to modify variables inside the debugged program.\n"
      "Examples: eval i=42\n"
      "          eval i++\n"
      "          eval x*=3.14159"
    },
    { "find", &DebuggerShell::cmd_find, 0,
      "addr pattern: scan memory page for a pattern of bytes; "
      "the pattern can be specified\n"
      "as a string formatted according to the following grammar:\n"
      "PATTERN  := BYTELIST\n"
      "BYTELIST := BYTE | BYTELIST BYTE\n"
      "BYTE     := 'CHAR' | HEXNUM | WILDCARD\n"
      "CHAR     := a-z | A-Z\n"
      "HEXNUM   := HEXDIGIT HEXDIGIT\n"
      "HEXDIGIT := 0-9 | a-f | A-F\n"
      "WILDCARD := ?\n"
      "Empty spaces that are not enclosed in single-quotes are ignored\n"
    },
    { "frame", &DebuggerShell::cmd_frame, 0,
       " <number>: Select a given stack frame and make it current. "
       "Subsequent\n"
       "commands will be executed in the context of this frame."
    },
    { "help", &DebuggerShell::cmd_help, auto_complete_command },
    { "handle", &DebuggerShell::cmd_handle, auto_complete_sig,
      " [stop][nostop][pass][nopass|ignore] <signum>: change signal handling.\n"
      "Tell the debugger to stop or not when signal occurs, and whether to pass\n"
      "the signal to the debugged program or to ignore it. If ignored, the debugged\n"
      "program will not see the signal -- note that this may break the program if\n"
      "it relies on a signal notification (such as SIGPOLL, for e.g.)."
    },
    { "instruction", &DebuggerShell::cmd_step, 0,
      ": execute one machine instruction (single-step program)."
    },
    { "loadcore", &DebuggerShell::cmd_loadcore,
       auto_complete_core, "load a core file in the debugger"
    },
    { "lookup", &DebuggerShell::cmd_lookup, auto_complete_sym,
      " [name [name]] [/c]: lookup symbols. If /c option is given (count),\n"
      "the command prints just the number of symbols that strictly match the names.\n"
      "Otherwise, each matching symbol is printed.\n"
    },
    { "open", &DebuggerShell::cmd_open, auto_complete_fname,
      " <filename>: open specified filename and redirect all subsequent\n"
      "console output to it, until a 'close' command is invoked.\n"
      "This may be useful when dumping large stack traces and symbol tables\n"
    },
    { "next", &DebuggerShell::cmd_next, 0,
      ": step program until the next line is reached,\n"
      "without diving into function calls."
    },
    { "print", &DebuggerShell::cmd_print, auto_complete_debug_sym,
      " [name [name] ]: print symbols in current scope; symbol names can be\n"
      "specified as arguments to this command; if no symbol name is given,\n"
      "all symbols in scope are printed."
    },
    { "line", &DebuggerShell::cmd_line, 0,
      "Print current line number\n"
    },
    { "list", &DebuggerShell::cmd_list, auto_complete_fname,
      " [<] [line-number] [how-many] [filename]: lists source file.\n"
      "'<' starts the listing at the beginning of the "
      "current function.\n"
      "'list 0' resets listing to the line number that\n"
      "corresponds to the current instruction pointer."
    },
    { "quit", &DebuggerShell::cmd_quit },

    { "restart", &DebuggerShell::cmd_restart, 0,
     ": restart the current program\n"
    },
    { "ret", &DebuggerShell::cmd_return, 0,
      ": step program until the current function returns\n"
    },
    { "run", &DebuggerShell::cmd_exec, auto_complete_fname,
      "[progname] [arg [arg]]: Start a program and attach to it.\n"
      "If the debugger is currently attached to another program,\n"
      "it automatically detaches before attaching to the new one.\n\n"
       "NOTE: This command behaves like exec, with the difference that the\n"
       "program name and all of its command line arguments are expanded\n"
       "according to shell rules. For example, run a.out 'ls'\n"
       "will execute the ls command, and pass its tokenized output as command\n"
       "line arguments to a.out\n"
    },
    { "set_next", &DebuggerShell::cmd_set_next, 0,
       "<file>:<line>|<addr>>\n"
       "Set the line or address to be executed next, by forcing the\n"
       "instruction pointer (aka program counter) to the specified value.\n"
    },
    { "symtab", &DebuggerShell::cmd_symtab, 0,
      " [/a] dump all symbol tables that are currently loaded\n"
      "sorted by demangled name (the default) or by address if\n"
      "/a or /addr is specified."
    },

    /* undocummented: "show regs /all" attempts to display FPU and XMM registers
      in addition to the general regs (the implementation needs reviewing and testing;
      "show threads /count" displays the number of threads */
    { "show", &DebuggerShell::cmd_show, auto_complete_show,
        " break | modules | regs | signals | threads\n"
        " list breakpoints/modules/registers/signal policies/threads"
    },
    { "step", &DebuggerShell::cmd_step, 0,
      ": execute on source line (i.e. step program until a different source line\n"
      "is reached). If a function call is encountered, dive into the function.\n"
    },
    { "thread", &DebuggerShell::cmd_switch_thread, auto_complete_thread,
      " <number>: switch view to the specified thread."
      "Subsequent commands (bt, where, frame, etc.) will happen in the\n"
      "context of the new thread."
    },
    { "up", &DebuggerShell::cmd_navigate_stack, 0, "move up the stack trace."
    },
    { "watch", &DebuggerShell::cmd_watch, auto_complete_sym,
      "<variable-name> [/w |/rw]: monitor memory accesses to a program variable.\n"
      "When an access occurs, the program will break in the\n"
      "debugger, and user-interactive mode will be entered.\n"
      "The /w flag tells the engine to break whenever the variable is being written.\n"
      "If no mode is specified, then /w will be assumed by default.\n"
      "The /rw (read-write) flags instruct the engine to stop on all accesses.\n"
      "Please NOTE that the debugger engine is using hardware debug registers for\n"
      "monitoring variables. On the x86 family, there can be as much as 4 (four)\n"
      "hardware breakpoints active at a given time."
    },
    { "whatis", &DebuggerShell::cmd_eval, auto_complete_debug_sym,
      " <symbol>|<expression>|<constant>: evaluate the given argument, which may\n"
      " be a variable's name, an expression, or a constant, and print its type.\n"
    },
    { "where", &DebuggerShell::cmd_where, 0,
      " [depth]: print backtrace, optional argument controls the depth of\n"
      "the backtrace (how many steps back to show); same as 'bt'"
    },

    { "yield", &DebuggerShell::cmd_yield, 0, ""
    },
};

#define ELEM_COUNT(x) sizeof(x)/sizeof((x)[0])

static const char prompt[] = "zero> ";


namespace DebugSymbolHelpers
{
    class ZDK_LOCAL Base : public DebugSymbolEvents
    {
        bool is_expanding(DebugSymbol*) const { return false; }

        void symbol_change(DebugSymbol*, DebugSymbol*) { }

    protected:

        BEGIN_INTERFACE_MAP(Base)
            INTERFACE_ENTRY(DebugSymbolEvents)
        END_INTERFACE_MAP()
    };

    /**
     * Print debug symbols to console
     */
    class ZDK_LOCAL Print : public Base
    {
        int numericBase_;

        bool notify(DebugSymbol* sym)
        {
            if (sym)
            {
                DebuggerShell::instance().print_debug_symbol(
                    sym, index_++, depth_, numericBase_);
            }
            return true;
        }
        int numeric_base(const DebugSymbol*) const
        {
            return numericBase_;
        }

    public:
        size_t index_;
        size_t depth_;

    public:
        Print() : numericBase_(10), index_(0), depth_(1) {}

        void set_numeric_base(int base) { numericBase_ = base; }
    };


    /**
     * Populate a vector of strings with the debug symbols
     * that match a string typed by the used.
     */
    class ZDK_LOCAL AutoComplete : public Base
    {
        const char* token_;
        size_t len_;
        vector<string>& matches_;

    public:
        AutoComplete(const char* token, vector<string>& matches)
            : token_(token), len_(0), matches_(matches)
        {
            if (token_) { len_ = strlen(token_); }
        }
        bool notify(DebugSymbol* sym)
        {
            assert(sym);
            if (strncmp(token_, sym->name()->c_str(), len_) == 0)
            {
                matches_.push_back(sym->name()->c_str());
            }
            return true;
        }
        int numeric_base(const DebugSymbol*) const
        {
            return 0;
        }
    };
}

namespace
{
    /**
     * Helper callback used by auto-complete; I cannot pass
     * a vector directly to DebuggerCommand::auto_complete,
     * because of interface rules.
     */
    class ZDK_LOCAL StringEnum : public EnumCallback<const char*>
    {
    public:
        explicit StringEnum(vector<string>& vs) : vs_(vs)
        {
            vs_.clear();
        }

        void notify(const char* s)
        {
            assert(s);
            vs_.push_back(s);
        }

    private:
        StringEnum(const StringEnum&);
        StringEnum& operator=(const StringEnum&);

        vector<string>& vs_;
    };
}


/**
 * Fill out MATCHES with all shell commands that match TEXT
 */
void DebuggerShell::auto_complete_command
(
    const char* text,
    vector<string>& matches
)
{
    assert(text);
    assert(matches.empty());

    if (matches.empty())
    {
        const size_t len = strlen(text);

        const CommandList& commands = instance().commands_;
        CommandList::const_iterator i = commands.begin();
        for (; i != commands.end(); ++i)
        {
            if (strncmp((*i)->name(), text, len) == 0)
            {
                matches.push_back((*i)->name());
            }
        }
    }
}


/**
 * Helper for the 'attach' command: find the program ids
 * that we can attach to, and that are a pontential match
 * (either by PID or the corresponding program name.
 */
static void
auto_complete_pid(const char* text, vector<string>& matches)
{
    class ZDK_LOCAL HelperCallback : public EnumCallback<const Runnable*>
    {
        const char* text_; // input
        size_t len_;
        vector<string>& matches_; // output

    public:
        HelperCallback(const char* text, vector<string>& matches)
            : text_(text)
            , len_(strlen(text))
            , matches_(matches)
        {}

        void notify(const Runnable* task)
        {
            ostringstream outs;

            outs << task->pid() << " (" << task->name() << ")";

            if (passwd* pwd = getpwuid( task->ruid() ))
            {
                outs << " [" << pwd->pw_name << "]";
            }
            if (strncmp(outs.str().c_str(), text_, len_) == 0
             || strncmp(task->name(), text_, len_) == 0)
            {
                matches_.push_back(outs.str());
            }
        }
    };
    assert(text);

    HelperCallback cb(text, matches);
    DebuggerShell::instance().enum_user_tasks(&cb);
}


/**
 * Match symbols from the currently debugged program against a
 * given string (the current word typed in the shell) and fill
 * out possible matches
 */
static void auto_complete_sym(const char* s, vector<string>& matches)
{
    assert(s);
    assert(matches.empty());

    const size_t len = strlen(s);

    const TargetManager& targets = DebuggerShell::instance();

    Lock<Mutex> lock(targets.mutex());
    TargetManager::const_iterator t = targets.begin(lock);
    const TargetManager::const_iterator end = targets.end(lock);
    for (; t != end; ++t)
    {
        SymbolEnum symEnum;
        (*t)->symbols()->enum_symbols(0, &symEnum);

        SymbolEnum::const_iterator i(symEnum.begin());
        for (; i != symEnum.end(); ++i)
        {
            const char* name = (*i)->name()->c_str();

            if (*s == 0)
            {
                matches.push_back(name);
            }
            else
            {
                const string symName((*i)->demangled_name(false)->c_str());

                if (strncmp(symName.c_str(), s, len) == 0)
                {
                    matches.push_back(symName);
                }
            }
        }
    }
}


static void
auto_complete_debug_sym(const char* s, vector<string>& matches)
{
    Debugger& debugger = DebuggerShell::instance();
    if (Thread* thread = debugger.get_thread(DEFAULT_THREAD))
    {
        DebugSymbolHelpers::AutoComplete expand(s, matches);
        debugger.enum_variables(thread, "", 0, &expand, LOOKUP_MODULE, true);
    }
    //auto_complete_sym(s, matches);
}


/**
 * Constructs a list of files in a given directory (path);
 * fills out the files vector.
 */
static void list_dir(
    const char*     path,
    vector<string>& files,
    const string&   pattern)
{
    struct stat st = { };

    Directory dir(path, (pattern + '*').c_str());

    Directory::const_iterator i = dir.begin();
    for (; i != dir.end(); ++i)
    {
        string tmp = *i;

        if (stat(tmp.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        {
            tmp += '/';
        }
        files.push_back(tmp);
    }
}


static void
auto_complete_fname(const char* fname, vector<string>& matches)
{
    assert(matches.empty());
    assert(fname);

    string path(fname);
    string pattern;

    const size_t n = path.rfind('/');
    if (n != string::npos)
    {
        pattern = path.substr(n + 1);
        path.erase(n);
    }

    try
    {
        list_dir(path.c_str(), matches, pattern);
    }
    catch (...)
    {
        // make sure we don't throw if path is not a directory
    }
    if (matches.empty())
    {
        pattern = fname;

        list_dir(".", matches, pattern);
    }
}


static void
auto_complete_core(const char* fname, vector<string>& matches)
{
    if (*fname == 0)
    {
        fname = "core";
    }
    auto_complete_fname(fname, matches);
}


namespace
{
    enum EnumBP
    {
        ENUM_BP_ALL,
        ENUM_BP_DISABLED,
        ENUM_BP_ENABLED,
    };

    /**
     * helper for auto_complete_breakpoints,
     * auto_complete_disabled_breakpoints, and
     * auto_complete_enabled_breakpoints
     */
    class ZDK_LOCAL BreakPointEnum
        : public EnumCallback<volatile BreakPoint*>
    {
    private:
        const char* tok_;
        size_t len_;
        vector<string>& matches_;
        EnumBP type_;

    public:
        BreakPointEnum(const char* tok, vector<string>& m, EnumBP type)
            : tok_(tok)
            , len_(strlen(tok))
            , matches_(m)
            , type_(type)
        { }

        void notify(volatile BreakPoint* bpnt)
        {
            assert(bpnt);
            if (bpnt->enum_actions("USER") == 0)
            {
                return;
            }
            if (type_ == ENUM_BP_DISABLED)
            {
                if (!has_disabled_actions(*bpnt))
                {
                    return;
                }
            }
            else if (type_ == ENUM_BP_ENABLED)
            {
            }
            ostringstream os;
            os << hex << showbase << CHKPTR(bpnt)->addr();

            if (len_ == 0)
            {
                matches_.push_back(os.str());
            }
            else if (strncmp(tok_, os.str().c_str(), len_) == 0)
            {
                matches_.push_back(os.str());
            }
        }
    };
} // namespace


static void
auto_complete_breakpoints(const char* tok, vector<string>& matches)
{
    assert(tok);

    BreakPointManager* mgr = DebuggerShell::instance().breakpoint_manager();
    if (mgr)
    {
        BreakPointEnum helper(tok, matches, ENUM_BP_ALL);
        mgr->enum_breakpoints(&helper);
    }
}


static void
auto_complete_disabled_breakpoints(const char* tok, vector<string>& matches)
{
    assert(tok);

    BreakPointManager* mgr = DebuggerShell::instance().breakpoint_manager();
    if (mgr)
    {
        BreakPointEnum helper(tok, matches, ENUM_BP_DISABLED);
        mgr->enum_breakpoints(&helper);
    }
}


static void
auto_complete_enabled_breakpoints(const char* tok, vector<string>& matches)
{
    assert(tok);
    BreakPointManager* mgr = DebuggerShell::instance().breakpoint_manager();

    if (mgr)
    {
        BreakPointEnum helper(tok, matches, ENUM_BP_ENABLED);
        mgr->enum_breakpoints(&helper);
    }
}


static void
auto_complete_thread(const char* tok, vector<string>& matches)
{
    const size_t len = tok ? strlen(tok) : 0;
    size_t numThreads = 0;

    const TargetManager& targets = DebuggerShell::instance();
    Lock<Mutex> lock(targets.mutex());

    TargetManager::const_iterator t = targets.begin(lock);
    const TargetManager::const_iterator end = targets.end(lock);
    for (; t != end; ++t)
    {
        numThreads += (*t)->enum_threads();
    }

    for (size_t n = 0; n != numThreads; ++n)
    {
        ostringstream buf;

        buf << n;
        if (strncmp(buf.str().c_str(), tok, len) == 0)
        {
            matches.push_back(buf.str());
        }
    }
}


/**
 * Some commands expect the thread to be non-NULL, and active;
 * check the conditions and throw exception if not satisfied.
 */
static void check_thread(Thread* thread)
{
    if (thread == 0)
    {
        throw runtime_error("No thread");
    }
    if (thread_finished(*thread))
    {
        throw runtime_error("Thread has exited");
    }
    assert(thread_is_attached(*thread));
}


/**
 * Get the current program counter
 */
static addr_t frame_program_count(Thread& thread)
{
    assert(thread.stack_trace());

    if (const Frame* frame = thread_current_frame(&thread))
    {
        return frame->program_count();
    }
    else
    {
        return thread.program_count();
    }
}


DebuggerShell& DebuggerShell::instance()
{
    assert(theDebugger_);
    return *theDebugger_;
}


void DebuggerShell::handle_signal(int signum, siginfo_t* info, void*)
{
#ifdef DEBUG
    if (info)
    {
        fprintf(stderr, "%s: signal %d, sender=%d instance=%p\n", __func__, signum, info->si_pid, theDebugger_);
    }
#endif

    if (theDebugger_)
    {
        theDebugger_->handle_signal_impl(signum);
    }
    else
    {
        _exit(signum);
    }
}



static const pthread_t __main__ = pthread_self();
static Mutex sigmutex;


static void bug_report()
{
#ifdef __linux__
    static char buf[1024];
    static void* trace[256];
    int nframes = backtrace(trace, 256);

    snprintf(buf, sizeof buf, "backtrace-%d.zero", getpid());
    auto_file f1(fopen(buf, "w"));
    auto_file f2(fopen("version.zero", "w"));
    int r;
    if (f1 && f2)
    {
        for (int i = 0; i < nframes; ++i)
        {
            fprintf(f1.get(), "%p\n", trace[i]);
        }
        fprintf(f2.get(), "%s\n", VERSION);
        f1.reset();
        f2.reset();
        static const char cmd[] =
    #if DEBUG
            "(cat /proc/%d/maps && echo \"%s\") > maps.zero";
    #else
            "(cat /proc/%d/maps && echo \"%s-release\") > maps.zero";
    #endif
        snprintf(buf, sizeof buf - 1, cmd, getpid(), SYSID);
        r = system(buf);
    }
    else
    {
        fprintf(stderr, "\nPlease send a BUG REPORT to bugs@zerobugs.org\n\n");
        fprintf(stderr, "build=%s\n", VERSION);
        fprintf(stderr, "--- backtrace ---\n");
        for (int i = 0; i < nframes; ++i)
        {
            fprintf(stderr, "%p\n", trace[i]);
        }
        snprintf(buf, sizeof buf, "cat /proc/%d/maps", getpid());
        r = system(buf);
    }
    (void)r;
#endif
}


void DebuggerShell::handle_signal_impl(int signo)
{
    static int interruptCount = 0;
    static time_t lastInterrupt = time(0);
    const time_t now = time(0);

    assert(signo);
    Lock<Mutex> lock(sigmutex);

    BlockSignalsInScope block(signo);
    try
    {
        pthread_t self = pthread_self();
        switch (signo)
        {
        case SIGINT:
            if (difftime(now, lastInterrupt) > 1)
            {
                interruptCount = 0;
            }
            else if (++interruptCount >= 3)
            {
                _exit(1);
            }
            lastInterrupt = now;
            set_signaled(true);
            if (!has_corefile())
            {
                stop();
            }
            break;

        case SIGALRM:
            set_signaled(true);
            if (!has_corefile())
            {
                stop();
            }
            break;

        default:
            // restore default handling for this signal so
            // that we don't loop forever
            signal(signo, SIG_DFL);

            if (self == __main__)
            {
                save_properties();

                // make sure we're detached from the debugged program
                // so that we don't leave it in an unstable state (with
                // breakpoints set, etc.)
                detach();
            }

            fprintf(stderr, "INTERNAL ERROR: signal=%d\n", signo);
            bug_report();

            // commit suicide, so that we can take a post-mortem
            // look at the core file and see what went wrong
            kill(0, signo);
        }
    }
    catch (const exception& e)
    {
        fprintf(stderr, "INTERNAL ERROR in handling signal=%d: %s\n",
            signo, e.what());
    }
    catch (...)
    {
        fprintf(stderr,
            "INTERNAL ERROR: unknown exception in handling signal=%d\n",
            signo);
    }
}



DebuggerShell::DebuggerShell()
    : disasmAddr_(0)
    , resume_(false)
    , current_(0)
    , promptLoopActive_(false)
    , disasmLineCount_(0)
{
    assert(theDebugger_ == 0);

    tcsetpgrp(0, getpgrp());

#ifdef USE_GNU_READLINE
    // initialize readline()
    // rl_catch_signals = 0; // do not use readline sig handlers
    rl_basic_word_break_characters = " \"\\''$><=;|&{(";

    // hook my auto-complete into readline
    rl_completion_entry_function = (rl_compentry_func_t*)(auto_complete);
#endif

    // initialize commands
    commands_.reserve(ELEM_COUNT(cmd_));
    for (size_t i(0); i != ELEM_COUNT(cmd_); ++i)
    {
        RefPtr<DebuggerCommand> ptr(new InternalCommand(cmd_[i]));
        commands_.push_back(ptr);
    }

    // initialize signals
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = handle_signal;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &sa, 0);
    sigaction(SIGSEGV, &sa, 0);
    sigaction(SIGTERM, &sa, 0);
    sigaction(SIGALRM, &sa, 0);

    if (env::get_bool("ZERO_CATCH_SIGABRT", true))
    {
        sigaction(SIGABRT, &sa, 0);
    }

    theDebugger_ = this;
}


DebuggerShell::~DebuggerShell() throw()
{
    assert(theDebugger_ == this);
    theDebugger_ = 0;
}


void DebuggerShell::print_banner() const
{
    struct utsname sysinfo;
    uname(&sysinfo);

    char buf[1024] = { 0 };

    snprintf(buf, sizeof(buf), 
        banner, ENGINE_MAJOR, ENGINE_MINOR, ENGINE_REVISION,
        sysinfo.sysname, sysinfo.release, sysinfo.machine,
        USER, HOSTNAME, copyright());

    cout << buf << endl;
}


void DebuggerShell::print_help(ostream& outs) const
{
    outs << "Usage: zero [pid | progname] [-option ...]" << endl;
    outs << "Options:" << endl;
    outs << " -v|--verbose  increase verbosity level" << endl;
    outs << " -h|--help     print this help" << endl;
    outs << " --ui-disable  run in text (console) mode" << endl;
    outs << endl;
    outs << "For help on interactive commands, type 'help' at ";
    outs << "the command the prompt." << endl;
    outs << "In GUI mode, check out the online help" << endl;
}



void DebuggerShell::set_current_thread(Thread* thread)
{
    current_ = thread;
    listing_.reset();
}


void DebuggerShell::resume(bool flag)
{
    resume_ = flag;
}


bool DebuggerShell::is_resumed() const
{
    return resume_;
}


void DebuggerShell::on_idle()
{
#ifdef DEBUG
    print_counted_objects(__PRETTY_FUNCTION__);
#endif
    prompt_user(0);
}


void DebuggerShell::begin_interactive_mode (
    Thread*     thread,
    EventType   eventType,
    Symbol*     sym)
{
    DebuggerEngine::begin_interactive_mode(thread, eventType, sym);
    prompt_user(thread, eventType);
}


/**
 * Called by the DebuggerBase when the debugged program
 * receives a signal, or when a thread exits.
 */
void DebuggerShell::on_event(Thread& thread)
{
    DebuggerEngine::on_event(thread);
}


/**
 * Helper wrapper around readline
 */
static string read_line(const char* prompt, bool addToHistory = true)
{
    static bool firstTime = true;
    if (firstTime)
    {
        firstTime = false;
        cout << "Type 'help' for a list of commands, ";
        cout << "<tab> to auto-complete.\n";
    }
    sigset_t signals;

    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);

    // BlockSignalsInScope block(signals);

    string result;

#ifdef USE_GNU_READLINE
    // Create a static instance of a class to
    // cleanup terminal settings after readline.
    static struct Cleanup
    {
        Cleanup() {}

        ~Cleanup() throw()
        {
            rl_cleanup_after_signal();
        }
    } __cleanup__;

    // GNU readline is not const-correct, we need to const_cast
    if (char* line = readline(const_cast<char*>(prompt)))
    {
        result = line;
        free(line);
    }
    if (addToHistory && !result.empty())
    {
        add_history(const_cast<char*>(result.c_str()));
    }
#else
    static ReadLine rl;

    rl.set_prompt(prompt);
    rl.set_auto_complete_func(DebuggerShell::auto_complete_impl);

    rl.read(result);

    if (addToHistory && !result.empty())
    {
        rl.add_history_entry(result);
    }
#endif // !USE_GNU_READLINE

    // trim trailing spaces
    while (size_t n = result.size())
    {
        if (result[--n] == ' ')
        {
            result.resize(n);
        }
        else
        {
            break;
        }
    }
    return result;
}


bool ZDK_EXPORT ptrace_error(int err)
{
    string msg("ptrace: ");
    msg += strerror(err);
    msg += " [C]ontinue or Abort? ";

    string resp = read_line(msg.c_str(), false);
    if (resp == "a" || resp == "A" || resp == "abort")
    {
        abort();
    }
    return false;
}



/**
 * Determine the event type based upon thread's state
 */
static EventType guess_event_type(Thread* thread)
{
    EventType event = E_PROMPT;

    if (thread)
    {
        event = thread_finished(*thread)
            ? E_THREAD_FINISHED : E_THREAD_STOPPED;
    }
    return event;
}


void DebuggerShell::print_return_value(const RefPtr<Thread>& thread)
{
    assert(thread);

    RefPtr<DebugSymbol> ret = thread->func_return_value();
    if (ret)
    {
        assert(ret->is_return_value());
        print_debug_symbol(ret.get(), 0, 1, 0);
    }
}


void DebuggerShell::show_listing(const RefPtr<Thread>& thread)
{
    vector<string> tmp;
    tmp.push_back("list");
    tmp.push_back("0");
    cmd_list(thread.get(), tmp);
    print_return_value(thread);
}


/**
 * Prompt the user for a command; if a plugin is interested
 * in handling the event, then the engine doesn't show the prompt,
 * assuming that the plugin will prompt the user instead.
 * This mechanism  allows plugins to implement custom command line
 * interface, or graphical user interfaces.
 * @note the first plugin that responds TRUE to the publish_event()
 * call will grab exclusive control (i.e. there can be only one active
 * prompt at a given time).
 */
void DebuggerShell::prompt_user(RefPtr<Thread> thread, EventType event)
{
    // guard against multiple re-entry
    assert(!promptLoopActive_);
    Temporary<bool> setFlag(promptLoopActive_, true);

    if (event == E_NONE)
    {
        event = guess_event_type(thread.get());
    }

    current_ = thread;

    // Prompt loop: read commands from console and execute them;
    // stay in the loop if command returns false, break otherwise.
    for (resume_ = false, disasmAddr_ = 0; !is_quitting();)
    {
        try
        {
            // publish event to all plugins
            const bool noPrompt = publish_event(thread.get(), event);
            set_silent(noPrompt);

            // if publish_event() returns false it means that no plugin
            // was interested in grabbing the event: we show the prompt,
            // read a line of input and process the command.

            if (!noPrompt && !is_quitting() && !resume_)
            {
                if (thread.get() && event != E_PROMPT)
                {
                    show_listing(thread);
                }
                const string cmdline = read_line(prompt);
                if (cmdline.empty())
                {
                    event = E_PROMPT; // keep prompting
                    continue;
                }
                addr_t tmpAddr = disasmAddr_;

                resume_ = dispatch_command(thread.get(), cmdline);

                // disasmAddr_ is used for resuming disassembly --
                // if it has not changed, reset it so that we keep
                // listing at the current instruction pointer
                // Q: should "help" be an exception to this?
                if (disasmAddr_ == tmpAddr)
                {
                    disasmAddr_ = 0;
                }
            }

            if (resume_)
            {
                dbgout(0) << __func__ << ": resuming event loop" << endl;

                listing_.reset();
                break;
            }

            // we are staying in the loop; the event that gets
            // passed to plugins in the next iteration is merely
            // a prompt rather than some thread event
            event = E_PROMPT;

            if (current_.get() && current_ != thread)
            {
                thread = current_;
                assert(thread);
            }
        }
        catch (const exception& e)
        {
            critical_error(thread.get(), e.what());

            event = E_PROMPT;

            if (signaled())
            {
                break;
            }
            quit();
        }

        if (!this->is_attached()) // detached from program?
        {
            current_ = thread = 0;
            break;
        }
        else if (!thread || thread_finished(*thread))
        {
            thread = get_thread(DEFAULT_THREAD);
            if (thread.get())
            {
                event = guess_event_type(thread.get());
                cout << "[" << thread->lwpid();
                cout << "]: is now current" << endl;
            }
        }
    } // end prompt loop
}


/**
 * Lookup command by name. The search is linear; the set of commands
 * is not very large, so it shouldn't be a problem
 */
DebuggerCommand* DebuggerShell::lookup_command(const string& name)
{
    DebuggerCommand* result = 0;

    CommandList::iterator i = commands_.begin();
    for (; i != commands_.end(); ++i)
    {
        if ((*i)->name() == name)
        {
            result = (*i).operator->();
            break;
        }
    }
    return result;
}


void DebuggerShell::auto_complete_impl (
    const char*     text,
    const string&   line,
    vector<string>& matches)
{
    typedef boost::char_separator<char> Delim;
    typedef boost::tokenizer<Delim> Tokenizer;

    Tokenizer tok(line, Delim(" "));

    string firstWord;

    if (tok.begin() != tok.end())
    {
        firstWord = *tok.begin();
    }

    DebuggerCommand* cptr = instance().lookup_command(firstWord);

    if (cptr == 0)
    {
        auto_complete_command(text, matches);
    }
    else
    {
        StringEnum stringEnum(matches);

        cptr->auto_complete(text, &stringEnum);
    }
}


#ifdef USE_GNU_READLINE
char* DebuggerShell::auto_complete(const char* text, size_t n)
{
    assert(text);
    static vector<string> matches;

    try
    {
        // readline is a C library:
        // this function must not throw C++ exceptions
        if (n == 0)
        {
            assert(rl_line_buffer);

            // initialize the list of possible matches
            auto_complete_impl(text, rl_line_buffer, matches);
        }
        if (matches.size() > n)
        {
            return strdup(matches[n].c_str());
        }
        matches.clear();
    }
    catch (const exception& e)
    {
        cerr << endl << __func__ << ": " << e.what() << endl;
    }
    return 0;
}
#endif


bool DebuggerShell::command(const char* cmd, Thread* thread)
{
    bool result = false;

    if (cmd)
    {
        result = dispatch_command(thread, cmd);
    }
    return result;
}


bool DebuggerShell::dispatch_command( Thread* thread, const string& cmd)
{
    assert(!cmd.empty());

    // break the line into a vector of tokens

    //boost::char_separator<char> delim(" ");
    typedef boost::escaped_list_separator<char> Delim;

    typedef boost::tokenizer<Delim> Tokenizer;

    Tokenizer tok(cmd, Delim('\\', ' ', '\"'));

    vector<string> argv;

    Tokenizer::const_iterator it = tok.begin();
    for (; it != tok.end(); ++it)
    {
        if (!(*it).empty())
        {
            argv.push_back(*it);
        }
    }

    // lookup the first token in the supported command
    DebuggerCommand* cptr = lookup_command(argv[0]);

    bool result = false;

    if (cptr == 0)
    {
        argv.insert(argv.begin(), "eval");
        cptr = lookup_command(argv[0]);
    }
    assert(cptr);

    vector<const char*> args(argv.size() + 1);
    transform(argv.begin(), argv.end(), args.begin(),
        mem_fun_ref(&string::c_str));

    assert(!args.empty());

    try
    {
        result = cptr->execute(thread, &args[0]);
    }
    catch (const exception& e)
    {
        cout << e.what() << endl;
    }

    // hack: typing "list" consecutively should continue
    // listing from where we left, other commands should
    // reset the listing to start at the line corresponding
    // to the current instruction pointer
    // Q: should "help" be an exception to this?
    if ((argv[0] != "list") && listing_.get())
    {
        listing_->set_current_line(0);
    }
    return result;
}


static void sym_to_addr(const RefPtr<Symbol>& sym, vector<addr_t>& addr, bool bpoint)
{
    ZObjectScope scope;
    if (!bpoint || sym->table(&scope)->addr())
    {
        addr.push_back(sym->addr());
    }
    else
    {
        sym->set_deferred_breakpoint(BreakPoint::GLOBAL);
    }
}


/**
 * Helper function. Whenever there is an ambiguity regarding the symbol
 * to which a command should apply, prompt the user with the possible
 * matches.
 */
static vector<addr_t> prompt_to_select_overload(
    const SymbolEnum& symbols,
    bool breakpoint,
    bool multipleSelectionOk)
{
    assert(!symbols.empty());
    vector<addr_t> addresses;

    RefPtr<Symbol> sym = *symbols.begin();

    for (;;)
    {
        if (symbols.size() == 1)
        {
            sym_to_addr(sym, addresses, breakpoint);
            break;
        }

        // Display a list of possible selections
        cout << "which " << sym->demangled_name(false)->c_str();
        cout << '?' << endl;
        cout << " [0] <escape>" << endl;

        for (size_t n(0); n < symbols.size(); ++n)
        {
            const RefPtr<Symbol>& s = symbols[n];

            cout << " [" << n + 1 << "] "
                 << s->demangled_name()
                 << " 0x" << setw(sizeof(addr_t) * 2) << setfill('0')
                 << hex << s->addr() << dec
                 << ' '   << s->file()->c_str()
                 << endl;
        }
        if (multipleSelectionOk)
        {
            cout << " [" << symbols.size() + 1 << "] <all>" << endl;
        }
        string line = read_line("Enter selection: ", false);
                                       //  no history ^
        if (line.empty() || !isdigit(line[0]))
        {
            continue;
        }
        size_t i = strtoul(line.c_str(), 0, 0);

        if (i == 0)
        {
            break;
        }
        else if ((--i) > symbols.size()
            || (i == symbols.size() && !multipleSelectionOk))
        {
            cout << "invalid entry, ";
            continue; // keep prompting for selection
        }
        else if (i == symbols.size())
        {
            // include all symbols in the result
            for (size_t n(0); n != i; ++n)
            {
                sym_to_addr(symbols[n], addresses, breakpoint);
            }
        }
        else
        {
            sym_to_addr(symbols[i], addresses, breakpoint);
        }
        break;
    }
    return addresses;
}


namespace
{
    class ZDK_LOCAL AddrCollector : public EnumCallback2<SymbolTable*, addr_t>
    {
    public:
        explicit AddrCollector(vector<addr_t>& addrs) : addrs_(addrs)
        {}

        void notify(SymbolTable*, addr_t addr) { addrs_.push_back(addr); }

    private:
        vector<addr_t>& addrs_;
    };
}

/**
 * A helper routine for reading addresses from the user's
 * command. addr_tes can be specified as decimal or hex
 * numbers, or by symbol (function) name. The thread's symbol
 * table is used to lookup the symbol names.
 */
static vector<addr_t> strings_to_addrs(
    const Thread& thread,
    const vector<string>& argv,
    const SourceListing* listing,
    bool breakpoint = false,
    bool multipleOk = true)
{
    assert(argv.size() > 1);
    vector<addr_t> result;  // return vector of addresses
    bool matchAll = false;

    vector<string>::const_iterator i = argv.begin();
    for (++i; i != argv.end(); ++i)
    {
        if (*i == "/all") // hack for break /all
        {
            matchAll = true;
            continue;
        }
        // check for file:line
        const size_t ncol = (*i).find(':');
        if ((ncol != string::npos) && ((*i)[ncol + 1] != ':'))
        {
            const size_t line = strtoul(i->c_str() + ncol + 1, 0, 0);
            RefPtr<SharedString> file(shared_string(i->c_str(), ncol));
            if (!file->length() && listing)
            {
                file = shared_string(listing->name());
            }
            AddrCollector collector(result);
            Debugger* debugger = CHKPTR(thread.debugger());
            if (!debugger->line_to_addr(file.get(), line, &collector, &thread))
            {
                cout << "Line not found: " << file.get();
                cout << ':' << line << endl;
            }
            continue;
        }

        // try reading the address as a number
        istringstream is(*i);
        is.unsetf(ios::basefield);
        addr_t addr = 0;
        is >> addr;
        if (addr)
        {
            result.push_back(addr);
        }
        else
        {
            // try it as a symbol name, enum symbols taking needed tables
            // into account (i.e. looking up shared objects that might be
            // loaded later)
            SymbolEnum syms;

            thread.symbols()->enum_symbols(
                i->c_str(), &syms,
                (SymbolTable::LKUP_DYNAMIC | SymbolTable::LKUP_UNMAPPED));

            if (syms.empty())
            {
                cout << "Function not found: " << *i << endl;
            }
            else if (matchAll)
            {
                SymbolEnum::const_iterator i = syms.begin();
                for (; i != syms.end(); ++i)
                {
                    result.push_back((*i)->addr());
                }
            }
            else
            {
                vector<addr_t> tmp =
                    prompt_to_select_overload(syms, breakpoint, multipleOk);
                result.insert(result.end(), tmp.begin(), tmp.end());
            }
        }
    }
    return result;
}


/**
 * add module -- useful for shared objects that are explicitly loaded with
 * the dlopen() system call. Because the linkage is not implicit, such modules
 * (or shared objects) are not listed in the DT_NEEDED section of the executable.
 */
bool DebuggerShell::cmd_addmod(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);

    assert(argv.size() >= 1);

    vector<string>::const_iterator i = argv.begin();
    for (++i; i != argv.end(); ++i)
    {
        CHKPTR(thread->symbols())->add_module((*i).c_str());
    }
    return false;
}


/**
 * Attach the debugger to a running program
 */
bool DebuggerShell::cmd_attach(Thread*, const vector<string>& argv)
{
    assert(argv.size() >= 1);

    for (size_t i = 1; i != argv.size(); ++i)
    {
        pid_t pid = strtol(argv[i].c_str(), 0, 0);

        if (pid == 0) continue;

        if (pid == getpid())
        {
            cout << "The Buddha says: Do not attach to self." << endl;
        }
        else
        {
            attach(pid);
        }
    }
    return false;
}


/**
 * Insert breakpoint at given address(es). Address can be given
 * explicitly (in hex) or as a function name.
 */
bool DebuggerShell::cmd_break(Thread* thread, const vector<string>& argv)
{
    if (argv.size() < 2)
    {
        cout << argv[0] << ": arguments missing" << endl;
        return false;
    }
    const bool matchAll = (argv[1] == "/all");
    check_thread(thread);

    vector<addr_t> addresses =
        strings_to_addrs(*thread, argv, listing_.get(), true, matchAll);

    vector<addr_t>::const_iterator i = addresses.begin();
    for (; i != addresses.end(); ++i)
    {
        if (set_user_breakpoint(get_runnable(thread), *i))
        {
            assert(thread->symbols());

            RefPtr<Symbol> sym(thread->symbols()->lookup_symbol(*i));
            if (sym.get())
            {
                StateSaver<ios, ios::fmtflags> save(cout);
                cout << "Breakpoint set at: 0x" << hex << sym->addr();
                cout << ' ' << sym->demangled_name()->c_str() << endl;
            }
        }
        else
        {
            cout << "There already is a breakpoint at: 0x";
            cout << hex << *i << dec << endl;
        }
    }
    return false;
}


/**
 * Remove a user breakpoint
 */
bool DebuggerShell::cmd_clear(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);

    for (size_t i = 1; i < argv.size(); ++i)
    {
        const size_t addr = strtoul(argv[i].c_str(), 0, 0);

    // NOTE at this time, this shell assumes that USER breakpoints are global;
    // per-thread USER breakpoints will be supported in the future

        if (remove_user_breakpoint(0, 0, addr))
        {
            cout << "breakpoint cleared: ";
        }
        else
        {
            cout << "No user breakpoint at: ";
        }
        cout << hex << addr << dec << endl;
    }
    return false;
}


bool DebuggerShell::cmd_open(Thread*, const vector<string>& argv)
{
    if (argv[0] == "close")
    {
        outputRedirect_.reset();
    }
    else if (argv.size() != 2)
    {
        cout << "Incorrect number of arguments, 1 expected\n";
    }
    else
    {
        for (int flags = O_RDWR | O_CREAT | O_EXCL;;)
        {
            bool fileExists = false;
            try
            {
                int fd = sys::open(argv[1].c_str(), flags);
                outputRedirect_.reset(new Redirect(1, fd, true));
                break;
            }
            catch (const SystemError& e)
            {
                if (e.error() == EEXIST)
                {
                    fileExists = true;
                }
                else
                {
                    cout << e.what() << endl;
                    break;
                }
            }
            if (fileExists)
            {
                string resp = read_line("File exists. Overwrite? [y/N]", false);
                if (resp == "y" || resp == "Y")
                {
                    flags = O_RDWR | O_TRUNC;
                }
                else
                {
                    break;
                }
            }
        }
    }
    return false; // stay in the command loop
}



/**
 * Resume debugged program. If a timer is specified, the debugged
 * program will be interrupted after the given number of milliseconds
 * elapses.
 */
bool DebuggerShell::cmd_continue(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);
    for (size_t i = 1; i < argv.size(); ++i)
    {
        if (argv[i] == "timer" && ++i < argv.size())
        {
            // milliseconds
            long msec = strtol(argv[i].c_str(), 0, 0);

            struct itimerval itv;
            memset(&itv, 0, sizeof itv);

            itv.it_value.tv_sec = msec / 1000;
            itv.it_value.tv_usec = (msec * 1000) % 1000000;

            setitimer(ITIMER_REAL, &itv, NULL);
        }
        else
        {
            cout << "Unknown argument: " << argv[i] << endl;
            return false;
        }
    }
    return true;
}


/**
 * Detach debugger from debuggee. If the debugged program was
 * started (exec-ed) by the debugger, it will be killed
 */
bool DebuggerShell::cmd_detach(Thread*, const vector<string>& argv)
{
    if (!is_attached())
    {
        cout << "Not attached." << endl; // Buddha
    }
    else
    {
        bool confirmed = false;
        for (size_t i = 1; i < argv.size(); ++i)
        {
            if (argv[i] == "/y")
            {
                confirmed = true;
            }
        }
        if (!confirmed)
        {
            // prompt user for confirmation
            string resp = read_line("Stop debugging? (y/n)", false);
            confirmed = (resp == "y" || resp == "Y" || resp == "yes");
        }
        if (confirmed)
        {
            detach();
        }
        else
        {
            return false;
        }
    }
    return true;
}


/**
 * Notification from the disassembler plugin.
 * @return true to keep disassembling, false to stop.
 */
bool DebuggerShell::notify(addr_t, const char* text, size_t)
{
    if (disasmLineCount_++ >= Term::screen_height(STDOUT_FILENO))
    {
        string resp = read_line("Continue or [Q]uit? ", false);
        if (resp == "q" || resp == "Q" || resp == "quit")
        {
            return false;
        }
        disasmLineCount_ = 0;
    }
    cout << text << endl;
    return true;
}


bool DebuggerShell::tabstops(size_t* first, size_t* second) const
{
    if (first)
    {
        *first = 2 * sizeof(addr_t) + 2;
    }
    if (second)
    {
        *second = 2 * sizeof(addr_t) + 32;
    }
    return true;
}



/**
 * Disassemble debugged program starting at current address.
 * Keep track of the address from one command to another so
 * that the next disassesmble command resumes from where we
 * had left (provided that no event happened since last disasm)
 */
bool DebuggerShell::cmd_disassemble(Thread* thread, const vector<string>& args)
{
    check_thread(thread);

    size_t howMany = 32;
    bool interleaved = true;

    vector<string>::const_iterator i(args.begin());

    // if the command name was not "disassemble" it means that
    // we got here as a failover from other command (probably a
    // "list" command which did not have source code available);
    // in this case, the command arguments do not apply
    //
    if (*i == "disassemble")
    {
        for (++i; i != args.end(); ++i)
        {
            if (*i == "at")
            {
                vector<string> argv(i, i + 2);
                // strings_to_addrs skips first arg
                vector<addr_t> alist =
                    strings_to_addrs(*thread, argv, NULL, false, true);
                disasmAddr_ = alist.empty() ? 0 : alist.front();
                ++i;
                if (i == args.end())
                {
                    break;
                }
            }
            else if (*i == "/nos") // no source
            {
                interleaved = false;
            }
            else
            {
                if (size_t n = strtoul(i->c_str(), 0, 0))
                {
                    howMany = n;
                }
            }
        }
    }
    if (disasmAddr_ == 0)
    {
        disasmAddr_ = frame_program_count(*thread);
    }
    const SymbolMap* symbols = CHKPTR(thread->symbols());
    RefPtr<Symbol> sym(symbols->lookup_symbol(disasmAddr_));
    if (sym)
    {
        ZObjectScope scope;
        if (const SymbolTable* table = sym->table(&scope))
        {
            dbgout(0) << table->filename()
                      << " base=0x"
                      << hex << table->adjustment()
                      << " addr=" << disasmAddr_
                      << endl;

            // round up to machine words
            const size_t nwords =
                (howMany + sizeof(word_t) - 1) / sizeof(word_t);

            vector<uint8_t> bytes(nwords * sizeof(word_t));

            try
            {
                word_t* buf = (word_t*) &bytes[0];
                thread->read_code(sym->addr(), buf, nwords);
            }
            catch (const exception& e)
            {
                cerr << __func__ << ": " << e.what() << endl;
            }

            disasmLineCount_ = 0;
            size_t n = disassemble( thread,
                                    sym.get(),
                                    howMany,
                                    interleaved,
                                    &bytes[0],
                                    this);
            disasmAddr_ += n;
        }
    }
    return false;
}


/**
 * Enable or disable existing breakpoints
 */
bool DebuggerShell::cmd_enable(Thread*, const vector<string>& argv)
{
    assert(!argv.empty());
    vector<string>::const_iterator i = argv.begin();
    const bool enable = (*i == "enable");

    for (++i; i != argv.end(); ++i)
    {
        istringstream is(*i);
        addr_t addr = 0;
        is >> addr;
        if (enable)
        {
            enable_user_breakpoint_actions(*this, addr);
        }
        else
        {
            disable_user_breakpoint_actions(*this, addr);
        }
    }
    return false;
}


/**
 * Execute (start) a program and attach to it. If currently
 * attached to another program, detaches automatically
 */
bool DebuggerShell::cmd_exec(Thread*, const vector<string>& argv)
{
    if (argv.size() > 1)
    {
        ExecArg args(argv.begin() + 1, argv.end());

        const bool shellExpandArgs = (argv[0] == "run");
        exec(args, shellExpandArgs, NULL);
    }
    return true;
}


void DebuggerShell::select_frame(Thread& thread, int n)
{
    if (StackTrace* trace = thread.stack_trace())
    {
        if (n >= 0 && trace->size())
        {
            if (Frame* frame = trace->selection())
            {
                if (frame->index() == static_cast<size_t>(n))
                {
                    return;
                }
            }
            trace->select_frame(n);

            // frame changed, reset address and listing
            disasmAddr_ = 0;
            listing_.reset();
        }

        if (trace->size())
        {
            if (Frame* frame = trace->selection())
            {
                cout << *frame << endl;

                vector<string> argv;
                argv.push_back("select_frame");
                cmd_line(&thread, argv);
            }
        }
    }
}


/**
 * Select a given stack frame and make it current. Subsequent
 * commands will be executed in the context of this frame
 */
bool DebuggerShell::cmd_frame(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);

    if (argv.size() <= 1)
    {
        select_frame(*thread, -1);
    }
    else if (argv.size() > 2)
    {
        cout << argv[0] << ": too many arguments" << endl;
    }
    else if (argv[1] == "signal")
    {
        // locate the signal frame
        if (StackTrace* stack = thread->stack_trace())
        {
            for (size_t i = 0; i != stack->size(); ++i)
            {
                if (stack->frame(i)->is_signal_handler())
                {
                    select_frame(*thread, i);
                    return false;
                }
            }
        }
        cout << "could not locate signal frame" << endl;
    }
    else
    {
        long f = strtol(argv[1].c_str(), 0, 0);
        select_frame(*thread, f);
    }
    return false; // do not resume execution
}


bool DebuggerShell::cmd_navigate_stack(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);

    if (argv.size() > 1)
    {
        cout << argv[0] << ": too many arguments" << endl;
    }
    else if (StackTrace* trace = thread->stack_trace())
    {
        if (trace->size())
        {
            if (Frame* frame = trace->selection())
            {
                size_t index = frame->index();
                if (argv[0] == "down")
                {
                    if (index)
                        --index;
                }
                else if (argv[0] == "up")
                {
                    ++index;
                }
                select_frame(*thread, index);
            }
        }
    }
    return false;
}


/**
 * Display help; if a command is given in the arguments, show
 * the help string that corresponds to that command
 */
bool DebuggerShell::cmd_help(Thread*, const vector<string>& argv)
{
    if (argv.size() == 1)
    {
        cout << "Supported commands:" << endl;

        vector<string> cmds;
        cmds.reserve(commands_.size());

        CommandList::const_iterator i = commands_.begin();
        for (; i != commands_.end(); ++i)
        {
            //cout << "  " << (*i)->name() << endl;
            cmds.push_back((*i)->name());
        }
        display_strings(cmds, stdout);
        cout << "Try 'help <command>' for more detailed info" << endl;
    }
    else
    {
        assert(argv.size() > 1);

        vector<string>::const_iterator i = argv.begin();
        for (++i; i != argv.end(); ++i)
        {
            DebuggerCommand* cptr = lookup_command(*i);
            if (cptr == 0)
            {
                cout << "No such command: " << *i << endl;
            }
            else
            {
                cout << cptr->name();

                if (const char* help = cptr->help())
                {
                    if (*help != ' ' && *help != ':')
                    {
                        cout << ": ";
                    }
                    cout << help << endl;
                }
                else
                {
                    cout << " -- no help available";
                }

                cout << endl;
            }
        }
    }
    return false;
}


/* auto-completes signal numbers */
static void auto_complete_sig(const char* text, vector<string>& matches)
{
    assert(text);
    const size_t len = strlen(text);

    static const char* args[] =
    { "pass", "nopass", "ignore", "stop", "nostop" };

    for (size_t i = 0; i != ELEM_COUNT(args); ++i)
    {
        if (strncmp(text, args[i], len) == 0)
        {
            matches.push_back(args[i]);
        }
    }

    const char* const* name = sig_name_list();
    for (; name && *name; ++name)
    {
        if (strncmp(text, *name, len) == 0)
        {
            matches.push_back(*name);
        }
    }
}


/**
 * Change the policy for handling a given signal
 */
bool DebuggerShell::cmd_handle(Thread*, const vector<string>& argv)
{
    assert(argv.size() >= 1);

    int passFlag = -1;
    int stopFlag = -1;

    unsigned sig = 0;

    for (size_t i = 1; i != argv.size(); ++i)
    {
        const string& arg = argv[i];

        if (arg == "stop")
        {
            stopFlag = 1;
        }
        else if (arg == "nostop")
        {
            stopFlag = 0;
        }
        else if (arg == "pass")
        {
            passFlag = 1;
        }
        else if (arg == "nopass" || arg == "ignore")
        {
            passFlag = 0;
        }
        else
        {
            sig = sig_from_name(arg);
        }
    }

    if (sig == 0)
    {
        cout << "Signal not specified" << endl;
    }
    else if (sig >= NSIG)
    {
        cout << "Invalid argument: " << sig << endl;
    }
    else
    {
        if (passFlag != -1)
        {
            signal_policy(sig)->set_pass(passFlag);
        }

        if (stopFlag != -1)
        {
            signal_policy(sig)->set_stop(stopFlag);
        }

        cout << "STOP\tPASS\tDESCRIPTION" << endl;
        cout << "----------------------------------------------";
        cout << endl;
        cout << *signal_policy(sig) << "\t" << sig_description(sig);
        cout << endl;
    }
    return false;
}


/* load a core file */
bool DebuggerShell::cmd_loadcore(Thread*, const vector<string>& argv)
{
    if (argv.size() > 1)
    {
        const char* prog = (argv.size() > 2) ? argv[2].c_str() : 0;
        load_core(argv[1].c_str(), prog);

        return true;
    }

    cout << "Core file not specified.\n";
    return false;
}


static bool sym_is_pointer(DebugSymbol* sym, bool* isString = 0)
{
    assert(sym);
    PointerType* ptr = interface_cast<PointerType*>(sym->type());
    if (ptr)
    {
        if (isString)
        {
            *isString = ptr->is_cstring();
        }
        return true;
    }
    return false;

}


void DebuggerShell::print_debug_symbol(
    DebugSymbol* sym,
    size_t index,
    size_t depth,
    int base,
    bool printSymName)
{
    if (!sym)
    {
        cout << "<null symbol>" << endl;
        return;
    }
    DebugSymbolHelpers::Print print;
    print.set_numeric_base(base);

    CHKPTR(sym)->read(&print);

    if ((sym->depth() > 1) && (index != 0))
    {
        cout << ", ";
    }
    if (printSymName)
    {
        cout << CHKPTR(sym)->name();
    }
    bool isLargeArray = false;
    if (ArrayType* aType = interface_cast<ArrayType*>(sym->type()))
    {
        isLargeArray = aType->elem_count() > max_array_range();
        cout << '[' << aType->elem_count() << ']';
    }
    if (sym->enum_children(0) && (!sym_is_pointer(sym) || sym->depth() < 2))
    {
        DebugSymbolHelpers::Print print;
        print.set_numeric_base(base);

        if (printSymName && CHKPTR(sym->name())->length())
        {
            cout << '=';
        }
        bool isString = false;
        if (sym_is_pointer(sym, &isString))
        {
            RefPtr<SharedString> value = sym->value();
            if (value)
            {
                if (isString)
                {
                    cout << value->c_str();
                    isString = true;
                }
                else
                {
                    cout << '[' << value->c_str() << "]=";
                }
            }
            else
            {
                cout << "[<null>]";
            }
        }
        if (!isString)
        {
            size_t index = 0;
            swap(index, print.index_);
            cout << '{'; ++print.depth_;
            if (isLargeArray)
            {
                cout << "...";
            }
            else
            {
                sym->enum_children(&print);
            }
            cout << '}';
            --print.depth_;
            print.index_ = index;
        }
    }
    else
    {
        if (sym->value())
        {
            if (printSymName && CHKPTR(sym->name())->length())
            {
                cout << '=';
            }
            RefPtr<SharedString> value = sym->value();
            if (value)
            {
                cout << value->c_str();
            }
            else
            {
                cout << "<null>";
            }
        }
    }
    if (depth == 1)
    {
        cout << endl;
    }
}


/**
 * Print symbols in current scope; symbol names can be specified as
 * arguments to this command; if no symbol name is given, all symbols
 * are printed
 */
bool DebuggerShell::cmd_print(Thread* thread, const vector<string>& argv)
{
    DebugSymbolHelpers::Print print;
    check_thread(thread);

    assert(thread->stack_trace());
    assert(thread->stack_trace()->selection());

    static const LookupScope scope = LOOKUP_MODULE;

    if (argv.size() == 1)
    {
        enum_variables(thread, "", 0, &print, scope, true);
    }
    else
    {
        static bool enumFuncs = false;

        for (size_t i = 1; i != argv.size(); ++i)
        {
            if (argv[i] == "/x")
            {
                print.set_numeric_base(16);

                if (i == 1 && argv.size() == 2)
                {
                    enum_variables(thread, "", 0, &print, scope, enumFuncs);
                }
            }
            else if (argv[i] == "/ret")
            {
                print_return_value(thread);
            }
            else if (!enum_variables(
                thread, argv[i].c_str(), 0, &print, scope, enumFuncs))
            {
                cout << "Symbol '" << argv[i] << "' not found.\n";
            }
        }
    }
    return false;
}


namespace
{
    /**
     * helper for cmd_reg, see below
     */
    class ZDK_LOCAL RegOut : public EnumCallback<Register*>
    {
        size_t i_;
        const size_t n_;

    public:
        explicit RegOut(size_t n) : i_(0), n_(n) { }

        void notify(Register* r)
        {
            if (i_++ == n_)
            {
                if (Variant* v = r->value())
                {
                    variant_print(cout, *v, 16);
                    cout << endl;
                }
            }
        }
    };
}


bool DebuggerShell::cmd_reg(Thread* thread, const vector<string>& argv)
{
    assert(argv[0] == "%r");

    if (argv.size() != 2)
    {
        cout << "Incorrect number of arguments, 1 expected\n";
    }
    else
    {
        check_thread(thread);
        const size_t n = strtoul(argv[1].c_str(), 0, 0);

        RegOut regOut(n);
        thread->enum_cpu_regs(&regOut);
    }
    return false;
}


namespace
{
    /**
     * Helper for DebuggerShell::cmd_eval, for async notifications
     */
    class ZDK_LOCAL ShellExprEvents : public SubjectImpl<ExprEvents>
    {
        int     base_;
        bool    whatis_; // print the type rather than the value?
        addr_t  addr_;

        ShellExprEvents(int base, bool whatis)
            : base_(base), whatis_(whatis), addr_(0)
        {}

        ShellExprEvents(const ShellExprEvents& other)
            : base_(other.base_)
            , whatis_(other.whatis_)
            , addr_(other.addr_)
        { }

    public:
        static RefPtr<ExprEvents> create(int base, bool whatis)
        {
            return new ShellExprEvents(base, whatis);
        }

        ////////////////////////////////////////////////////////
        virtual ~ShellExprEvents() throw()
        { }

        ////////////////////////////////////////////////////////
        bool on_done(Variant* v, bool*, DebugSymbolEvents*)
        {
            if (v)
            {
                StateSaver<ios, ios::fmtflags> save(cout);

                if (base_ == 16)
                {
                    cout << hex << showbase;
                }
                else if (base_ == 8)
                {
                    cout << oct << showbase;
                }
                if (DebugSymbol* sym = v->debug_symbol())
                {
                    if (whatis_)
                    {
                        cout << CHKPTR(sym->type_name()) << endl;
                    }
                    else
                    {
                        DebuggerShell& shell = DebuggerShell::instance();
                        shell.print_debug_symbol(sym, 0, 1, base_, false);
                    }
                }
                else
                {
                    if (whatis_)
                    {
                        cout << variant_type(v->type_tag());
                    }
                    else
                    {
                        variant_print(cout, *v);
                    }
                    cout << endl;
                }
            }
            return true;
        }

        ////////////////////////////////////////////////////////
        void on_error(const char* errMsg)
        {
            cout << errMsg << endl;
        }

        ////////////////////////////////////////////////////////
        void on_warning(const char* errMsg)
        {
            cout << errMsg << endl;
        }

        /**
         * Handle events that occur while interpreting an
         * expression
         */
        bool on_event(Thread* thread, addr_t addr)
        {
            const int sig = thread->signal();

            if (addr_ && sig && (addr != addr_))
            {
                ostringstream msg;
                msg << sig_name(thread->signal());
                msg << " occurred at " << hex << showbase << addr;
                msg <<" while interpreting expression in lwpid=";
                msg << dec << thread->lwpid();

                on_error(msg.str().c_str());

                if (sig == SIGTRAP)
                {
                    return false;
                }
                else
                {
                    thread->set_signal(0);
                    Runnable& task = interface_cast<Runnable&>(*thread);
                    task.set_program_count(addr_);
                }
            }
            return true;
        }

        ////////////////////////////////////////////////////////
        void on_call(addr_t addr, Symbol* symbol)
        {
            if (symbol)
            {
                addr_ = addr; // entering function call at addr_
            }
            else
            {
                addr_ = 0; // function call returning
            }
        }

        ExprEvents* clone() const
        {
            return new ShellExprEvents(*this);
        }
    };
}


/**
 * Evaluate a C++ expression.
 */
bool DebuggerShell::cmd_eval(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);

    int base = 0; // numeric base
    string expr;

    assert(argv.size() >= 1);
    for (size_t i = 1; i != argv.size(); ++i)
    {
        if (i == 1 && argv[i] == "/x")
        {
            base = 16;
            continue;
        }
        else
        {
            expr += argv[i];
            expr += ' ';
        }
    }
    const bool whatis = (argv[0] == "whatis");
    RefPtr<ExprEvents> events = ShellExprEvents::create(base, whatis);

    return !evaluate(expr.c_str(), thread, 0, events.get(), base);
}


/**
 * dump memory
 */
bool DebuggerShell::cmd_dump(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);

    addr_t beginAddr = 0;
    addr_t endAddr = 0;
    vector<string>::const_iterator i = argv.begin();
    for (++i; i != argv.end(); ++i)
    {
        if (beginAddr == 0)
        {
            beginAddr = strtoul((*i).c_str(), 0, 0);
        }
        else if (endAddr == 0)
        {
            endAddr = strtoul((*i).c_str(), 0, 0);
        }
        else
        {
            cout << "Address range already specified\n";
        }
    }
    if (beginAddr && (endAddr == 0))
    {
        endAddr = beginAddr + 64;
    }
    cout << hex << beginAddr << "-" << endAddr << dec << endl;
    if (beginAddr > endAddr)
    {
        string resp = read_line("Invalid range, ok to swap? (y/n)", false);
        if (resp == "y" || resp == "Y" || resp == "yes")
        {
            std::swap(beginAddr, endAddr);
        }
    }
    if (beginAddr <= endAddr)
    {
        dump(cout, *thread, beginAddr, endAddr);
    }
    return false;
}


void DebuggerShell::dump(
    ostream& outs,
    Thread& thread,
    addr_t beginAddr,
    addr_t endAddr)
{
    assert(endAddr >= beginAddr);

    const bool isStdout = (&outs == &cout);

    StateSaver<ios, ios::fmtflags> saveState(outs);

    if (endAddr > beginAddr)
    {
        const size_t nbytes = (endAddr - beginAddr);
        size_t nwords = (nbytes + sizeof(word_t) - 1) / sizeof (word_t);
        vector<word_t> buf(nwords);

        size_t wordsRead = 0;

        thread.read_data(beginAddr, &buf[0], nwords, &wordsRead);

        const size_t cols = isStdout
            ?  (Term::screen_width(STDOUT_FILENO)
                - sizeof(addr_t) * 2 - 5) / 4
            : 16;

        for (size_t n = 0, rows = 0; n < nbytes; ++rows)
        {
            if (isStdout && (rows + 1 >= Term::screen_height(STDOUT_FILENO)))
            {
                string resp = read_line("Continue or [Q]uit? ", false);
                if (resp == "q" || resp == "Q" || resp == "quit")
                {
                    break;
                }
                rows = 0;
            }

            outs << hex << setw(sizeof(addr_t) * 2);
            outs << setfill('0') << beginAddr + n << ": ";
            {
                Temporary<size_t> save(n);
                for (size_t j = 0; (j != cols); ++j, ++n)
                {
                    if (n < nbytes)
                    {
                        const int b =
                            ((const unsigned char*)&buf[0])[n];
                        outs << setw(2) << setfill('0') << b << ' ';
                    }
                    else
                    {
                        outs << "   ";
                    }
                }
            }
            outs << "  ";
            for (size_t j = 0; (j != cols) && (n < nbytes); ++j, ++n)
            {
                const int b = ((const char*)&buf[0])[n];
                outs << (isprint(b) ? (char)b : '.');
            }
            outs << endl;
        }
    }
}


/**
 * Find bytes in memory
 */
bool DebuggerShell::cmd_find(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);

    string bytes;
    addr_t addr = 0;
    size_t len = 0;

    vector<string>::const_iterator i = argv.begin();
    for (++i; i != argv.end(); ++i)
    {
        if (addr == 0)
        {
            addr = strtoul((*i).c_str(), 0, 0);
        }
        else
        {
            if (!bytes.empty())
            {
                bytes += ' ';
            }
            bytes += *i;
        }
    }
    dbgout(0) << "search pattern=\"" << bytes << "\"" << endl;

    if (addr == 0)
    {
        cout << "No memory address specified.\n";
    }
    else if (bytes.empty())
    {
        cout << "No byte pattern specified.\n";
    }
    else if (thread_page_find(*thread, addr, bytes.c_str(), &len, &addr))
    {
        dump(cout, *thread, addr, addr + len);
    }
    else
    {
        cout << "Byte pattern not found.\n";
    }
    return false;
}


/**
 * print current line info
 */
bool DebuggerShell::cmd_line(Thread* thread, const vector<string>& argv)
{
    if (!listing_.get())
    {
        cmd_list(thread, argv);
    }
    else
    {
        if (argv[0] != "line")
        {
            cout << listing_->name() << ':';
        }
        cout << listing_->symbol_line() << endl;
    }
    return false;
}


/**
 * List source file, or disassemble if source is not available
 */
bool DebuggerShell::cmd_list(Thread* thread, const vector<string>& args)
{
    string filename;
    size_t line = 0;
    size_t symbolLine = 0;
    size_t howManyLines = 0;
    bool lineSpecified = false;
    bool startAtFun = false;

    // inspect command arguments
    vector<string>::const_iterator i = args.begin();
    for (++i; i != args.end(); ++i)
    {
        assert(!(*i).empty());

        if (isdigit((*i)[0]))
        {
            const size_t n = strtoul((*i).c_str(), 0, 0);
            if (line)
            {
                howManyLines = n;
            }
            else
            {
                line = n;
                if (n)
                {
                    lineSpecified = true;
                }
                else //if (listing_.get())
                {
                    //listing_->set_symbol_line(0);
                    listing_.reset();
                }
            }
        }
        else if (*i == "<")
        {
            startAtFun = true;
        }
        else
        {
            filename = canonical_path(i->c_str());
        }
    }
    if (filename.empty() && !startAtFun)
    {
        if (listing_.get())
        {
            filename = listing_->name();
            if (line == 0)
            {
                line = listing_->symbol_line();
            }
        }
    }
    if (filename.empty())
    {
        check_thread(thread);

        if (const Frame* frame = thread_current_frame(thread))
        {
            ZObjectScope scope;
            // the current function in scope
            RefPtr<Symbol> func = frame->function();
            if (!func || !func->line() || !func->table(&scope))
            {
                listing_.reset();
                // no source file available, fallback to disassembly
                return cmd_disassemble(thread, args);
            }
            if (startAtFun)
            {
                addr_t addr = func->addr() - func->offset();
                SymbolTable* table = CHKPTR(func->table(&scope));
                if (RefPtr<Symbol> s = table->lookup_symbol(addr))
                {
                    func = s;
                    lineSpecified = true;
                }
            }
            filename = CHKPTR(func->file())->c_str();
            symbolLine = func->line();
            if (!lineSpecified)
            {
                line = symbolLine;
            }
        }
    }
    if ((listing_.get() == 0) || (listing_->name() != filename))
    {
        listing_.reset(new SourceListing(filename));
    }
    if (listing_->empty())
    {
        return cmd_disassemble(thread, args);
    }
    // start listing from specified line
    if (lineSpecified || listing_->current_line() == 0)
    {
        listing_->set_current_line(line);
    }
    if (symbolLine)
    {
        listing_->set_symbol_line(symbolLine);
    }
    if (howManyLines == 0)
    {
        howManyLines = 20;
    }
    const size_t next = listing_->list(cout, howManyLines);
    listing_->set_current_line(next);
    return false; // do not resume execution
}


bool DebuggerShell::cmd_lookup(Thread* thread, const vector<string>& args)
{
    check_thread(thread);
    vector<string>::const_iterator i = args.begin();

    bool verbose = true;
    SymbolEnum symEnum;

    for (++i; i != args.end(); ++i)
    {
        if (*i == "/c")
        {
            verbose = false;
        }
        else if ((*i)[0] == '/')
        {
            cout << "invalid option: " << *i << endl;
        }
        else
        {
            SymbolMap* symMap = CHKPTR(thread->symbols());
            symMap->enum_symbols((*i).c_str(), &symEnum);
        }
    }
    if (verbose)
    {
        SymbolEnum::const_iterator i = symEnum.begin();
        for (; i != symEnum.end(); ++i)
        {
            cout << *i << endl;
        }
    }
    else
    {
        cout << symEnum._count() << endl;
    }
    return false;
}


/**
 * Execute a source line, not diving into function calls
 */
bool DebuggerShell::cmd_next(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);

    const addr_t addr = frame_program_count(*thread);
    if (argv.size() > 1)
    {
        cout << argv[0] << ": too many arguments" << endl;
    }
    return next(thread, addr, 0);
}


bool DebuggerShell::cmd_quit(Thread*, const vector<string>&)
{
    try
    {
        quit();
    }
    catch (const exception& e)
    {
        cerr << __func__ << ": " << e.what() << endl;
        _exit(1);
    }
    return true; // keep compiler happy
}


/**
 * STEP debugged program until the current function returns
 */
bool DebuggerShell::cmd_return(Thread* thread, const vector<string>&)
{
    check_thread(thread);
    get_runnable(thread)->step_until_current_func_returns();

    return true; // exit the prompt_user() loop
}


namespace
{
    ostream& operator<<(ostream& outs, WatchType type)
    {
        switch(type)
        {
        case WATCH_WRITE: return outs << "write";
        case WATCH_READ_WRITE: return outs << "read-write";
        case WATCH_VALUE: assert(false);
        }
        return outs;
    }
}


/**
 * Manipulate watchpoints -- a watchpoint is similar to a breakpoint,
 * only it gets activated when the program is accessing a variable,
 * rather than fetching an instruction.
 */
bool DebuggerShell::cmd_watch(Thread* thread, const vector<string>& argv)
{
    check_thread(thread); // needs a valid thread

    WatchType type = WATCH_WRITE;
    string variable;
    vector<string>::const_iterator i = argv.begin();
    for (++i; i != argv.end(); ++i)
    {
        if (*i == "/rw")
        {
            type = WATCH_READ_WRITE;
        }
        else if (*i == "/r")
        {
            type = WATCH_WRITE;
        }
        else if ((*i)[0] == '/')
        {
            cout << "invalid option: " << *i << endl;
        }
        else
        {
            if (!variable.empty())
            {
                cout << "*** Warning: variable already specified, overriding\n";
            }
            variable = *i;
        }
    }
    if (variable.empty())
    {
        cout << "Variable not specified. You are a bozo.\n";
    }
    else
    {
        DebugSymbolEnum debugSyms;

        if (enum_variables( thread,
                            variable.c_str(),
                            0,
                            &debugSyms,
                            LOOKUP_MODULE,
                            false))
        {
            DebugSymbolEnum::const_iterator i = debugSyms.begin();

            for (; i != debugSyms.end(); ++i)
            {
                dbgout(0) << (*i)->name() << "=" << (void*)(*i)->addr()
                          << endl;

                if (!set_watchpoint(get_runnable(thread), type, true, (*i)->addr()))
                {
                    cout << "error setting watchpoint at: ";
                    cout << (*i)->name() << endl;
                }
            }
        }
        else
        {
            cout << "No such symbol: " << variable << endl;
        }
    }
    return false;
}


/**
 * Show stack trace
 */
bool DebuggerShell::cmd_where(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);
    size_t maxDepth = UINT_MAX;
    bool currentFunction = false;

    if (argv.size() > 2)
    {
        cout << "Extra arguments ignored" << endl;
    }
    else if (argv.size() > 1)
    {
        if (argv[1] == "/func")
        {
            currentFunction = true;
            maxDepth = 1;
        }
        else if (size_t n = strtoul(argv[1].c_str(), 0, 10))
        {
            maxDepth = n;
        }
    }
    RefPtr<SymbolMap> syms = thread->symbols();
    RefPtr<StackTrace> trace = thread->stack_trace(maxDepth);

    // number of lines in the output, plus room for prompt
    size_t height = 1;
    const size_t w = Term::screen_width(STDOUT_FILENO);

    // screen_width should never return 0, by contract
    assert(w);

    maxDepth = min(maxDepth, trace->size());
    for (size_t i = 0; i != maxDepth; ++i)
    {
        Frame* frame = trace->frame(i);
        assert(frame);

        ostringstream buf;
        if (currentFunction)
        {
            RefPtr<Symbol> func = frame->function();
            buf << func->demangled_name();
        }
        else if (frame->is_signal_handler())
        {
            buf << " #" << i << " 0x" << hex;
            buf << setw(2 * sizeof(addr_t)) << setfill('0');
            buf << frame->program_count() << dec;
            buf << " (SIGNAL HANDLER)";
        }
        else
        {
            RefPtr<Symbol> func = frame->function();
            buf << " #" << i << ' ';
            print_symbol(buf, frame->program_count(), func);
        }

        // compute the height (i.e. the number of lines on
        // the screen needed to print this frame)
        const size_t h = (buf.str().length() + w - 1) / w;

        cout << buf.str() << endl;
        height += h;
        if (height >= Term::screen_height(STDOUT_FILENO))
        {
            string resp = read_line("Continue or [Q]uit? ", false);
            if (resp == "q" || resp == "Q" || resp == "quit")
            {
                break;
            }
            height = 1; // new screen
        }
    }
    cout << endl;
    return false;
}


////////////////////////////////////////////////////////////////
// 
// For automated tests only -- don't read too much into it ;)
//
bool DebuggerShell::cmd_yield(Thread* thread, const vector<string>& argv)
{
    long seconds = 0;

    for (size_t i = 0; i != argv.size(); ++i)
    {
        if (argv[i][0] == '-')
        {
        }
        else
        {
            seconds = strtol(argv[i].c_str(), 0, 0);
        }
    }
    alarm( seconds );
    return true;
}

////////////////////////////////////////////////////////////////
static void auto_complete_show(const char* text, vector<string>& matches)
{
    assert(text);
    const size_t len = strlen(text);

    static const char* args[] =
    {
        "breakpoints",
        "files",
        "modules",
        "regs",
        "signals",
        "status",
        "threads"
    };

    for (size_t i = 0; i != ELEM_COUNT(args); ++i)
    {
        if (strncmp(text, args[i], len) == 0)
        {
            matches.push_back(args[i]);
        }
    }
}


bool DebuggerShell::cmd_set_next(Thread* thread, const vector<string>& argv)
{
    if (argv.size() < 2)
    {
        cout << argv[0] << ": arguments missing" << endl;
        return false;
    }
    check_thread(thread);
    if (Runnable* task = interface_cast<Runnable*>(thread))
    {
        vector<addr_t> addr =
            strings_to_addrs(*thread, argv, listing_.get());

        if (addr.size() > 1)
        {
            cout << argv[0] << ": to many arguments\n";
        }
        task->set_program_count(addr.front());
        show_listing(thread);
    }
    else
    {
        cout << "the operation cannot be performed on core files\n";
    }
    return false;
}


/**
 * Step one line
 */
bool DebuggerShell::cmd_step(Thread* thread, const vector<string>& argv)
{
    check_thread(thread);
    bool machineInstrLevel = (argv.front() == "instruction");

    const addr_t addr = frame_program_count(*thread);
    if (argv.size() > 1)
    {
        cout << argv[0] << ": too many arguments" << endl;
    }

    step(thread, addr, machineInstrLevel);
    return true; // resume debuggee
}


/**
 * Dump symbol tables
 */
bool DebuggerShell::cmd_symtab(Thread* thread, const vector<string>& args)
{
    struct ZDK_LOCAL Dumper : public EnumCallback<SymbolTable*>
                            , public EnumCallback<Symbol*>
    {
        SymbolTable::LookupMode mode_;

        // sort by demangled names by default
        Dumper() : mode_(SymbolTable::LKUP_DEMANGLED) { }

        void notify(SymbolTable* table)
        {
            StateSaver<ios, ios::fmtflags> saveState(cout);

            for ( ; table; table = table->next())
            {
                if (table->size())
                {
                    cout << "***** " << table->filename();
                    cout << " " << table->name() << " *****\n";

                    table->enum_symbols(NULL, this, mode_);
                    cout << endl;
                }
            }
        }

        void notify(Symbol* symbol)
        {
            if (symbol)
            {
                cout << hex << setw(2 * sizeof(addr_t)) << symbol->addr();
                cout << ": " << symbol->demangled_name() << endl;
            }
        }
    } dumper;


    check_thread(thread);

    vector<string>::const_iterator i = args.begin();
    for (++i; i != args.end(); ++i)
    {
        if ((*i == "/a") || (*i == "/addr"))
        {
            dumper.mode_ = SymbolTable::LKUP_SYMBOL;
        }
    }

    thread->symbols()->enum_symbol_tables(&dumper);

    return false;
}


namespace
{
    /**
     * Function object for use with Thread::enum_user_regs.
     * Prints the register to an output stream.
     */
    class ZDK_LOCAL RegisterPrinter
        : public EnumCallback<Register*>
        , public EnumCallback3<const char*, reg_t, reg_t>

    {
        ostream& outs_;

    public:
        explicit RegisterPrinter(ostream& outs) : outs_(outs)
        { }

        void notify(Register* reg)
        {
            if (reg)
            {
                StateSaver<ios, ios::fmtflags> save(outs_);
                outs_ << reg->name() << "\t\t";
                outs_ << hex << showbase;

                RefPtr<Variant> v = reg->value();
                variant_print(outs_, *v) << endl;

                if (reg->enum_fields(this))
                {
                    outs_ << endl;
                }
            }
        }

        void notify(const char* name, reg_t value, reg_t)
        {
            outs_ << ' ' << name << '=' << value;
        }
    };
}


namespace // Helper Callbacks
{
    class ZDK_LOCAL ModuleEnumerator : public EnumCallback<Module*>
    {
        bool unloaded_;

    public:
        explicit ModuleEnumerator(bool unloaded = false)
            : unloaded_(unloaded) { }

        void notify(Module* module)
        {
            if (module)
            {
                pid_t pid = 0;

                if (SymbolTable* symtab = module->symbol_table_list())
                {
                    // todo: should also check for symtab->adjustment()?
                    if (unloaded_ && (symtab->addr() || symtab->upper()))
                    {
                        return;
                    }
                    ZObjectScope scope;
                    if (Process* proc = symtab->process(&scope))
                    {
                        pid = proc->pid();
                    }
                }
                StateSaver<ios, ios::fmtflags> save(cout);
                cout << "[" << setw(7) << setfill(' ') << pid << "] ";
                cout << hex << setfill('0');
                cout << setw(sizeof(addr_t) * 2);
                cout << module->addr() << '-';
                cout << setw(sizeof(addr_t) * 2);
                cout << module->upper();
                cout << ": " << module->name() << endl;
            }
        }
    };


    class ZDK_LOCAL FileEnumerator : public EnumCallback2<int, const char*>
    {
        void notify(int fd, const char* filename)
        {
            cout << setw(5) << setfill(' ');
            cout << fd << ' ' << filename << endl;
        }
    };


    class ZDK_LOCAL ThreadPrinter : public EnumCallback<Thread*>
    {
        ostream& out_;
        Thread* current_;
        size_t  count_;

    public:
        ThreadPrinter(ostream& out, Thread* current)
            : out_(out), current_(current), count_(0)
        { }

        void notify(Thread* thread)
        {
            out_ << setw(5) << count_++ << ' ' << *thread;

            if (thread->single_step_mode())
            {
                out_ << " single-stepping";
            }
            if (thread == current_)
            {
                out_ << " <";
            }
            out_ << endl;
        }
    };
} // namespace



bool DebuggerShell::cmd_show(Thread* thp, const vector<string>& argv)
{
    if (argv.size() < 2)
    {
        cout << "Missing argument." << endl;
    }
#ifdef DEBUG
    else if (argv[1] == "heaps")
    {
        cout << "heap<16>: used: " << Fheap<16>::used_bytes();
        cout << " avail: " << Fheap<16>::free_bytes() << endl;
    }
#endif
    else if (argv[1] == "signals")
    {
        unsigned int n = 32;
        if ((argv.size() > 2) && (argv[2] == "/all"))
        {
            n = NSIG;
        }
        cout << "STOP\tPASS\tDESCRIPTION" << endl;
        cout << "-----------------------------------------------\n";

        for (unsigned i = 0; i != n; ++i)
        {
            cout << *signal_policy(i) << "\t" << sig_description(i)
                 << endl;
        }
    }
    else if (argv[1] == "threads")
    {
        if ((argv.size() > 2) && (argv[2] == "/count"))
        {
            cout << enum_threads(NULL) << endl;
        }
        else
        {
            ostringstream buf;
            ThreadPrinter callback(buf, thp);

            size_t count = enum_threads(&callback);
            if (count)
            {
                cout << "       PID  S  PPID   GID    ID\n";
                cout << "--------------------------------------\n";
                cout << buf.str();
            }
        }
    }
    else if (argv[1] == "breakpoints")
    {
        pid_t pid = 0;
        if (argv.size() > 2)
        {
            pid = strtoul(argv[2].c_str(), 0, 0);
        }
        print_breakpoints(cout, pid);
    }
    // we need a running thread for breakpoints and regs
    else if (thp == 0)
    {
        cout << "No current thread" << endl;
    }
    else if (argv[1] == "stat")
    {
        print_event_info(cout, *thp);
    }
    else if (argv[1] == "regs")
    {
        RegisterPrinter regprn(cout);
        if (argv.size() > 2 && argv[2] == "/all")
        {
            thp->enum_cpu_regs(&regprn);
        }
        if (argv.size() > 2 && argv[2] == "/debug")
        {
            interface_cast<ThreadImpl&>(*thp).dump_debug_regs(cout);
        }
        else
        {
            thp->enum_user_regs(&regprn);
        }
        cout << endl;
    }
    else if (argv[1] == "modules")
    {
        bool unloaded = false;
        if (argv.size() > 2 && argv[2] == "/unloaded")
        {
            unloaded = true;
        }
        ModuleEnumerator modEnum(unloaded);
        enum_modules(&modEnum);
    }
    else if (argv[1] == "files")
    {
        FileEnumerator fileEnum;
        if (Runnable* task = interface_cast<Runnable*>(thp))
        {
            task->enum_open_files(&fileEnum);
        }
    }
    else if (argv[1] == "status")
    {
        if (SharedString* descr = thread_get_event_description(*thp))
        {
            cout << descr->c_str() << endl;
        }
        else
        {
            cout << sig_description(thp->signal());
        }
    }
    else
    {
        cout << "Unknown argument: " << argv[1] << endl;
        vector<string> args;

        args.push_back("help");
        args.push_back("show");

        cmd_help(thp, args);
    }
    return false;
}


bool DebuggerShell::cmd_switch_thread(Thread*, const vector<string>& argv)
{
    if (argv.size() < 2)
    {
        cout << argv[0] << ": arguments missing" << endl;
    }
    else if (argv.size() > 2)
    {
        cout << argv[0] << ": too many arguments" << endl;
    }
    else
    {
        size_t n = strtoul(argv[1].c_str(), 0, 0);

        // first, try it as a lwpid
        if (Thread* thread = get_thread(n))
        {
            set_current_thread(thread);
        }
        else
        {
            // helper callback, get the n-th thread
            class Callback : public EnumCallback<Thread*>
            {
                size_t n_, count_;

            public:
                RefPtr<Thread> thread_;

                explicit Callback(size_t n) : n_(n), count_(0) { }

                virtual ~Callback() { }

                void notify(Thread* thread)
                {
                    if (count_++ == n_)
                    {
                        thread_ = thread;
                    }
                }
            };

            Callback callback(n);

            const TargetManager& targets = DebuggerShell::instance();
            Lock<Mutex> lock(targets.mutex());

            const TargetManager::const_iterator end = targets.end(lock);
            for (TargetManager::const_iterator t = targets.begin(lock); t != end; ++t)
            {
                (*t)->enum_threads(&callback);
            }

            if (!callback.thread_)
            {
                cout << argv[0] << ": no such thread: " << n << endl;
            }
            else
            {
                set_current_thread(callback.thread_.get());
            }
        }
    }
    return false; // do not resume execution
}


#ifdef DEBUG_OBJECT_LEAKS
bool DebuggerShell::cmd_count_objects(Thread*, const vector<string>&)
{
    print_counted_objects(__func__);
    return false;
}
#endif



/**
 * Overrides PluginManager, looks for plugins that implement the
 * DebuggerCommand interface (i.e. plugins that may extend the set
 * of DebuggerShell commands).
 */
bool DebuggerShell::on_interface(
    DynamicLibPtr   lib,
    uuidref_t       iid,
    Unknown2*&      component)
{
    try
    {
        if (!DebuggerEngine::on_interface(lib, iid, component))
        {
            return false;
        }
        if (DebuggerCommand* cmd = interface_cast<DebuggerCommand*>(component))
        {
            if (cmd->name())
            {
                commands_.push_back(cmd);
            }
        }
    }
    catch (const exception& e)
    {
        cerr << "DebuggerShell::on_interface: " << e.what() << endl;
        return false;
    }
    return true;
}



void DebuggerShell::add_command(DebuggerCommand* cmd)
{
    commands_.push_back(cmd);
    DebuggerEngine::add_command(cmd);
}


/**
 * Restart last process with the same command line args
 * and environment variables
 */
bool DebuggerShell::cmd_restart(Thread* thread, const vector<string>& argv)
{
    const char* cmd = NULL;
    const char* const* env = NULL;

    if (thread)
    {
        RefPtr<Process> proc = thread->process();
        if (proc)
        {
            env = proc->environment();
            if (SharedString* commandLine = proc->command_line())
            {
                cmd = commandLine->c_str();
            }
        }
        if (!cmd)
        {
            cmd = thread->filename();
        }
    }
    else if (const HistoryEntry* entry = get_most_recent_history_entry())
    {
        env = entry->environ();

        if (entry->is_live())
        {
            cmd = entry->command_line();
        }
        if (!cmd)
        {
            cmd = entry->name();
        }
    }
    else
    {
        cout << "could not get most recent target\n";
    }
    if (cmd)
    {
        exec(cmd, false, env);
    }
    return true;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
