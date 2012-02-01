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

/* Design Notes

Architecture
--------------------------------------------------------------------------------
The overall design is based on an "object model". The object model is a
collection of abstractions, each modeling an artifact involved in debugging
a program. Example of such artifacts include, but are not limited to: Thread,
BreakPoint, Symbol, SymbolTable; these abstractions are expressed in C++ as
abstract classes. Abstract classes are the C++ way of coding interfaces.

    The object model is a collection of interfaces, each modeling an artifact
    related to debugging.

The debugger has a modular, component-based architecture. The central part is
a layered engine core that provides basic debugger functionality, such as
execution control, breakpoint management, and symbol table management.

SIDE NOTE
    The supported symbol format is ELF (Executable and Linkable Format), used by
    the Linux operating system. The ELF format supports core files, created by the
    operating system when a program crashes. Consequently, the debugger supports
    loading core files, and post-mortem examination of crashed programs.

The three layers of the engine core implement most of the abstract classes in
the object model.

The engine core provides minimal support for interaction with the user, via
a command line interface, manages breakpoints (inserted by both the user and
the debugger itself for internal purposes) and controls the execution of the
debuggee threads. Additional functionality is provided by plug-in components.

Fig 1. Debugger architectural blocks

ENGINE CORE                                                 PLUG-INS

+----------------------------------------------------+      +-------------------+
|                                                    |      |                   |
| DebuggerShell -- controls interaction with the user|      | DebugInfo readers |
|                                                    |      |                   |
+-----------------------+----------------------------+      +-------------------+
                        |
                        |
+-----------------------V----------------------------+      +-------------------+
|                                                    |      |                   |
| DebuggerEngine -- manages plugins and breakpoints  |      | Front End (GUI)   |
|                                                    |      |                   |
+-------+---+-----------+----------------------------+      +-------------------+
        |   |           |
        |   |           |
+-----------------------V----------------------------+      +-------------------+
|                                                    |      |                   |
| DebuggerBase -- manages debuggee threads;          |      | Disassembler      |
|                 execution control                  |      |                   |
+----------------------------------------------------+      +-------------------+
        |   |
        |   +-------------------+
        |                       |
+-------+-------+       +-------+----------+                +-------------------+
| ELF           |       | C/C++ expression |                |                   |
| SymbolTable   |       | evaluation       |                | Heap Monitor      |
| management    |       | (interpreter)    |                |                   |
+---------------+       +------------------+                +-------------------+


As shown in Figure 1., support for the STABS and DWARF debug format, a GUI, a
disassembler and a heap monitor are currently implemented as separate
plug-ins.

The engine notifies all plug-ins about crucial events such as the loading of a
dynamic library, the creation of a new thread inside of the debugged program,
or the receipt of a signal. When the debugged program receives a signal, the
event is "broadcasted" to the plug-ins. Normally, after the plugins have been
notified about the signal, the engine displays a prompt at the console and waits
for the user to type a command. A plug-in can however override this user interaction,
and implement either a custom prompt (possibly extended with a scripting
language) or a GUI.

This architecture favors ease of upgrade and extensibility. Plug-ins can be
deployed separately from the engine. Users can extend the debugger with their
own plug-ins.

Interface example:

DECLARE_INTERFACE_(BreakPointAction, ZObject)
{
    DECLARE_UUID("b5717f44-4218-461c-9577-2c27f87534bd")

    virtual const char* name() const = 0;

     // Executes the action when the breakpoint is hit.
     // If the method returns false, the action is removed
     // from the breakpoint list of actions (and it is not
     // executed again when the breakpoint is hit). If all
     // actions are removed, the breakpoint itself is erased.
     //
    virtual bool execute(Thread*, BreakPoint*) = 0;

    // ...
    // ...
};

SIDE NOTE
	The DECLARE_INTERFACE_ macro ensures the interoperability between modules
	(plugins and or main engine) built with GCC 2.95 and modules built with GCC 3.x
	(or another C++ ABI Version 3 compatible compiler, such as Intel Compiler 8).

An important part of the DebuggerEngine layer is the breakpoint manager.

Breakpoints are central to the DebuggerEngine layer; breakpoints can be
set by the user, or by the debugger for internal purposes (such as detecting
the creation of new threads).

Breakpoints
--------------------------------------------------------------------------------

Physical vs. Logical

Breakpoints can be classified in several ways. One categorization
distinguishes between "logical breakpoints" and "physical breakpoints". What
this means is that not all the breakpoints that you have inserted in the
program are physically there, but the debugger will support the illusion that
they are; reality is the realm of physical breakpoints, and logic is derived
off perception. So if you perceive a breakpoint as being inserted in the
debugged process, it is logically there, even though, physically, the debuggee
has not been affected.

Let's consider a couple of examples, to help bring the discussion out of the
philosophical realm.

1) The user inserts a breakpoint at the beginning of a function that is not loaded
into memory yet, because it lives in a shared library that has not been mapped
into the debugge's memory space -- just yet.

The debugger nicely shields the user from knowing such details, and may say:
"OK. I don't know what the address in memory of function `abc' is; but I know that it
is implemented inside the dynamic library libabc.so; I will keep this in mind, so that
if I later detect that libabc.so is loded, I will insert the breakpoint. "

2) Another case may be that the debugger has inserted a breakpoint at the beginning of
the pthread_create() function, to internally keep track of newly created threads.

The user wants to insert a breakpoint at the same address, and does not need to know
that a physical breakpoint is already there.

The debugger associates two logical actions with the same physical breakpoint: one that
internally updates the list of debugged threads, and another one that initiates an
interaction with the user.

The logical breakpoints are implemented as actions associated with physical breakpoints.
Each physical breakpoint maintains a list of actions. An action may be temporary (or
once-only), which means that it gets discarded after being executed once. Once-only
actions are similar to UNIX System V signal handlers. Non-temporary actions are
executed each time the physical breakpoint is hit -- similar to BSD signal handlers.

Algorithm for executing breakpoint actions
------------------------------------------

// Execute actions on given thread
void BreakPoint::execute_actions(Thread* thread)
{
    // The list of actions associated with this breakpoint
    // may change during the execution of actions, and thus
    // the iterators may be invalidated; make a copy of the
    // actions and cycle thru the copy, to be safe.
    ActionList tmp(this->actions_);

    ActionList::iterator i = tmp.begin();
    for (size_t d = 0; i != tmp.end(); ++d)
    {
        if (is_disabled(*i))
        {
            ++i;
            continue;
        }
        // a temporary action returns false
        if ((*i)->execute(thread, this))
        {
            ++i;
        }
        else
        {
            // remove it from the master list
            ActionList::iterator j = actions_.begin();
            advance(j, d);

            actions_.erase(j);

            // remove it from tmp as well so that
            // destruction is not delayed
            i = tmp.erase(i);
        }
    }
}


Software vs. Hardware

The Intel 386/486/585/686 family of chips offers support for debugging,
including breakpoints. The CPU has 6 debug registers: 4 for addresses, one for
control, and one for status. Each of the first 4 can hold a memory address
that causes a hardware fault when accessed.

SIDE NOTE
    In Intel lingo, a "fault" is a hardware notification, or event, that
    happens when the CPU is about to access a memory address -- that is,
    before the access happens. An "exception" is a similar notification, only
    that it happens after the access has occurred.

    REMEMBER: Hardware breakpoints are faults, software breakpoints are
    exceptions.

The control register holds some flags that specify the type of access (read,
read-write, execute) and some other bits; the status register is
helpful for determining which breakpoint was hit, when handling a system
fault.

Thanks to Operating System magic, the hardware breakpoints are multiplexed,
so we can have as many as N times 4 hardware breakpoints per program, where
N is the number of threads in the program.

Hardware breakpoints have the advantage of being non-intrusive -- the debugged
program is not modified. Another advantage is that they can be set to monitor
data as well as code. A debugger may use a hardware breakpoint to detect that
a memory location is being accessed.

Software breakpoints are implemented as a special opcode (INT 0x3) that is
inserted in the code at location to be monitored.

SIDE NOTE
    Nicely enough, Intel has a dedicated opcode for breakpoints. Other
    CPUs (PowerPC, for example) do not have a special opcode; on those
	platforms software breakpoints are implemented by inserting an invalid code
	at the desired location.

Software breakpoints have the main drawbacks of being slow and intrusive. The
debugged program has to be modified, and the debugger needs to memorize
the original opcode at the modified location, so that the debuggee's code is
restored when the breakpoint is removed.

When a software breakpoint is hit, the instruction pointer needs to be
decremented, and the original opcode restored. Then the debugged program has
to be stepped out of the breakpoint. After the breakpoint is handled, the
breakpoint opcode is reinserted.

On UNIX derivatives (such as Linux), a debugger does not manipulate the debugged
program directly; rather, it uses the operating system as a middle man (via
the ptrace or /proc API). This implies that every time the debugger reads or
writes into the debuggee's memory space, a context switch from user mode to
kernel mode happens.

Another disadvantage of soft breakpoints is that they can only monitor code.
Software breakpoints cannot be used for watching data accesses.

What makes software breakpoints indispensable is that there's no limit to how
many can be inserted. Hardware breakpoints are a very scarce resource (you can
run out of the 4 of them quite fast); software breakpoints are intrusive and
slower, but can be used abundently.

The design decision in my debugger is to use software breakpoints for
user-specified breakpoints, and prefer hardware breakpoints for internal
purposes. Watchpoints (breakpoints that monitor data access) are implemented as
hardware breakpoints.

An example of breakpoints maintained by the debugger internally is stepping
over function calls. A breakpoint is inserted at the location where the
function returns, and control is given to the debuggee to run at full speed
until the breakpoint is hit. The breakpoint is removed once it is hit, and the
hardware resource can then be reused.

As a rule of thumb, the debugger employs the hardware support for cases where
breakpoints are expected to be released after relatively short amounts of time.

If no hardware registers are avaialable, the debugger falls back to using a
software breakpoint.


Global vs. Per-Thread

Another categorization of breakpoints is by the what threads they affect in a
multi-threaded program. A global breakpoint causes the program to stop,
regardless of what thread has hit it. Per-thread breakpoints will stop the
program only when reached by a given thread. Because all threads share the
same code segment, a software breakpoint is also a global breakpoint, since
any thread that reaches the break opcode will stop.

The operating system creates the illusion of each thread running on its own
CPU, therefore a hardware breakpoint may be private to a given thread.

SIDE NOTE
    A bit in the debug control register of the 386 chip can be used to control
    the global/per-task behavior of hardware breakpoints.

A thread ID can be added to the data structure or class that represents a software
breakpoint. When the breakpoint is hit, the thread ID in the structure may be
compared against the ID of the current thread. The behavior of a per-thread breakpoint
can be emulated this way.

The debugger uses emulated breakpoints when it needs a hardware breapoint
and none of the 4 debug registers is available.

Consider the case where the debugger uses a breakpoint for quickly stepping
over function calls. The debugged program must stop only if the breakpoint at
the function's return address is hit by the same thread that was current when
the user gave the "step over" command.


Thread Management
--------------------------------------------------------------------------------
The DebuggerBase layer is built around an event loop, somewhat similar to a
modern GUI system.

Events occur and get dispatched to their proper handlers from within a main
loop. The difference is that in the debugger's case, the events do not come
from the keyboard or the mouse, but from the debugged program.

Basically it boils down to:

    while (!quit_)
    {
        get_event();
        handle_event();
        resume_threads();
    }

There are two types of events: 1) a debugged thread has received a signal and
2) a debugged thread has exited. The debugger uses the waitpid() system call
to figure what event occurred.

SIDE NOTE
    Although similar in concept, the debugger's event loop is not the same
    with the GUI event loop. In my implementation, the GUI (or front-end) runs
    a separate event loop in its own thread. The DebuggerBase instance runs
    the back-end loop in another thread. When an event occurs, the back-end
    notifies the front-end, and then blocks until a command is posted back
    from the GUI. There are two event loops: one is debuggee-driven, the other
    one is user-driven.

    Main event loop in the DebuggerBase layer
    -----------------------------------------
    while ( !quit_ )
    {
        if (threads_.empty())
        {
            wait_for_threads();
        }
        else if (has_corefile())
        {
            handle_corefile();
        }
        else
        {
            RefPtr<Thread> tptr;

            try
            {
                tptr = get_event();
            }
            catch (const exception& e)
            {
                // attempt to re-stabilize
                // ...
            }
            // handle the event unless it was caused
            // by the debugger-sent STOP signal
            if (!tptr->is_stopped_by_debugger())
            {
                handle_event(tptr);
            }
        }
        if (!has_corefile())
        {
            resume_threads();
        }
    }

As shown above, there are some extra details that need to be handled
in the event loop, such as error conditions and making sure that we are
reporting events that are actually caused by the interaction between the
debugger and the debuggee.

When the debugger needs to stop the execution of the debuggee (the user gave the
"break" command), it sends it a SIGSTOP signal, and marks all the threads as
"requested to stop".
When the signal is received by the debugged program, the debugger is notified
(the waitpid() call returns).

FOOT NOTE
    For more on the mechanics of signals and waitpid() check out the manual pages
    for signal, waitpid, and ptrace.

The debugger checks the threads to see if they were previously marked, to
determine whether the signal came in response to a stop request.

NOTE 1
There are a few cases where the state of a thread is collected with waitpid(),
from other places in the engine's code than the main loop; in these cases, the
thread pointers are saved into a queue, so that the event gets handled as soon
as control goes back to the main loop.


NOTE 2
The RefPtr template in the code above is an intrusive reference-counting, smart pointer.
Most interfaces in the object model are derived off the base interface ZObject, which
provides a) intrusive reference counting and b) dynamic type casting. b) is needed
because of a compiler hack that makes GCC 2.95-built vtables compatible with a GCC 3-built
vtables; dynamic_cast<> is not available when building with GCC-2.95. The ZObject interface
provides a query_interface method that needs to be used instead. I wrote an interface_cast<>
template around the query_interface() method -- in the future, when I drop GCC-2.95 support,
interface_cast may change to simply call dynamic_cast.


The Expression Interpreter
--------------------------------------------------------------------------------
A builtin interpreter allows the user to evaluate simple, one-liner C/C++ expressions.

No variables can be declared, only constants and variables from the debugged program are
allowed. Function calls are also allowed, if out-of-line code has been generated.

Functions that are inlined or optimized out cannot be used in expressions. Simmilarly,
variables that are optimized out by the compiler cannot be referenced in debugger expressions.

The GUI allows expressions to be entered as conditions for activating a breakpoint.
The debugger stops at a conditional breakpoint only when the expression evaluates to true
(non-zero).

The built-in interpreter can perform type casts (useful when examining objects via pointer-to-base)
and can call functions inside the debugged program.

*/
#include "zdk/config.h"
#include "zdk/log.h"
#include "zdk/init.h"
#include <iostream>
#include <string>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include "dharma/canonical_path.h"
#include "dharma/environ.h"
#include "dharma/pipe.h"
#include "dharma/sigutil.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"
#include "dharma/task_pool.h"
#include "generic/singleton.h"

using namespace std;

bool ptrace_error(int);

static jmp_buf ret_from_segv;
static void ignore_segv(int)
{
    longjmp (ret_from_segv, 1);
}


static bool detect_vmware()
{
    uint32_t magic = 0;

#if __linux__ && (__x86_64__ || __i386__)
    sighandler_t old = signal(SIGSEGV, ignore_segv);
    if (setjmp(ret_from_segv) == 0)
    {
        uint32_t verMajor, verMinor, dout;
        __asm__ __volatile__ (
             "mov $0x564D5868, %%eax;"  // magic number
             "mov $0x0, %%ebx;"         // random number, but not magic
             "mov $0x0000000A, %%ecx;"  // specifies command
             "mov $0x5658, %%edx;"      // VMware I/O port */
             "in %%dx, %%eax;"
             "mov %%eax, %0;"
             "mov %%ebx, %1;"
             "mov %%ecx, %2;"
             "mov %%edx, %3;" : "=r"(verMajor), "=r"(magic), "=r"(verMinor), "=r"(dout)
        );
    }
    signal(SIGSEGV, old);
#endif
    return (magic == 0x564D5868);
}


static void set_plugin_path(const char* filename)
{
    string path = canonical_path(filename);

    size_t n = path.rfind('/');
    if (n != string::npos)
    {
        path.erase(n);
        path += "/../plugin";
        path = canonical_path(path.c_str());
    }
    setenv("ZERO_PLUGIN_PATH", path.c_str(), false);
}


// for pthread_atfork
static Mutex fMutex(false); // non-recursive

static void acquire()
{
    fMutex.enter();
}
static void release()
{
    fMutex.leave();
}


int main(int argc, char* argv[])
{
/* 
    http://pubs.opengroup.org/onlinepubs/007904975/functions/pthread_atfork.html

    "There at least two serious problems with the semantics of fork() in a multi-threaded program.
    One problem has to do with state (for example, memory) covered by mutexes. Consider the case
    where one thread has a mutex locked and the state covered by that mutex is inconsistent whilex
    another thread calls fork(). In the child, the mutex is in the locked state (locked by a nonexistent
    thread and thus can never be unlocked). Having the child simply reinitialize the mutex is
    unsatisfactory since this approach does not resolve the question about how to correct or otherwise
    deal with the inconsistent state in the child." */

    if (int err = pthread_atfork(acquire, release, release))
    {
        cerr << "pthread_atfork failed: " << strerror(err) << endl;
    }

    try
    {
        sys::set_ptrace_error_handler(ptrace_error);

        if (env::get_bool("ZERO_SOURCE_STEP", true))
        {
            setenv("LD_BIND_NOW", "1", 1);
        }
        if (detect_vmware())
        {
            clog << "VMware detected, turning off hardware breakpoints\n";
            setenv("ZERO_HARDWARE_BREAKPOINTS", "0", 1);
        }
        set_plugin_path(argv[0]);

        Debugger* dbg = debugger_init(argc, argv);

        if (dbg)
        {
            // force instantiation while still single-threaded
            Singleton<TaskPool>::instance();

            return debugger_run(dbg) ? 0 : 1;
        }
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
    }
    return 2;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
