//
// $Id: breakpoint.cpp 729 2010-10-31 07:00:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <boost/utility.hpp>
#include "zdk/autocontext.h"
#include "zdk/check_ptr.h"
#include "zdk/switchable.h"
#include "zdk/thread_util.h"
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "dharma/syscall_wrap.h"
#include "generic/state_saver.h"
#include "generic/temporary.h"
#include "dbgout.h"
#include "thread.h"
#include "breakpoint.h"


using namespace std;
using namespace eventlog;


////////////////////////////////////////////////////////////////
BreakPointBase::BreakPointBase
(
    RefPtr<Thread>  thread,
    addr_t          addr,
    int             enable
)
  : thread_(thread)
  , addr_(addr)
  , enable_(enable)
{
}


////////////////////////////////////////////////////////////////
BreakPointBase::BreakPointBase
(
    RefPtr<Thread>  thread,
    RefPtr<Symbol>  sym,
    addr_t          addr
)
  : thread_(thread)
  , sym_(sym)
  , addr_(addr)
  , enable_(0)
{
}


////////////////////////////////////////////////////////////////
BreakPointBase::BreakPointBase
(
    const BreakPointBase& other,
    Thread& thread
)
  : thread_(&thread)
  , sym_(NULL) // FIXME: lookup_symbol may not work for deferred
               // breakpoints
  , addr_(other.addr_)
  , enable_(other.enable_)
  , actions_(other.actions_)
{
}


////////////////////////////////////////////////////////////////
BreakPointBase::~BreakPointBase() throw()
{
}


////////////////////////////////////////////////////////////////
Thread* BreakPointBase::thread() const volatile
{
    assert(thread_);
    return thread_.get();
}


////////////////////////////////////////////////////////////////
void BreakPointBase::reparent(const RefPtr<Thread>& thread) volatile
{
    if ((thread.get() != this->thread()) && thread)
    {
        const_cast<RefPtr<Thread>&>(thread_) = thread;
    #ifdef DEBUG
        clog << __func__ << ": " << thread->lwpid() << endl;
    #endif
    }
}


/**
 * Cast away the volatile qualifer
 */
inline RefPtr<Symbol>& volatile_cast(volatile RefPtr<Symbol>& sym)
{
    return const_cast<RefPtr<Symbol>&>(sym);
}

////////////////////////////////////////////////////////////////
Symbol* BreakPointBase::symbol() const volatile
{
    if (!sym_ && thread())
    {
        if (Frame* frame = thread_current_frame(thread()))
        {
            if (frame->program_count() == addr_)
            {
                volatile_cast(sym_) = frame->function();
            }
        }
        if (!sym_)
        {
            if (SymbolMap* symbols = thread()->symbols())
            {
                volatile_cast(sym_) = symbols->lookup_symbol(addr_);
            }
        }
    }
    return sym_.get();
}


////////////////////////////////////////////////////////////////
bool BreakPointBase::is_enabled() const volatile
{
    return enable_ > 0;
}


////////////////////////////////////////////////////////////////
int BreakPointBase::enable() volatile
{
    bool success = true;
    Temporary<int> save(enable_);

    if (enable_++ == 0)
    {
        success = do_enable(true);
    }
    if (success)
    {
        save.set_value(enable_);
    }
    return enable_;
}


////////////////////////////////////////////////////////////////
int BreakPointBase::disable() volatile
{
    assert(thread());

    bool success = true;
    Temporary<int> save(enable_);

    if (enable_-- == 1)
    {
        success = do_enable(false);
    }
    if (success)
    {
        save.set_value(enable_);
    }
    return enable_;
}


////////////////////////////////////////////////////////////////
void BreakPointBase::debug_enable(bool onOff, Thread* thread) volatile
{
#if DEBUG
    if (!thread)
    {
        thread = CHKPTR(this->thread());
    }
    if (!thread)
    {
        return;
    }
    if (Debugger* debugger = thread->debugger())
    {
        DebugChannel channel(__func__, debugger->verbose());
        eventlog::Stream<DebugChannel> log(channel, 1);

        RefPtr<SymbolMap> symbols(thread->symbols());
        RefPtr<Symbol> sym(CHKPTR(symbols)->lookup_symbol(addr()));

        log << (onOff ? "enabling " : "disabling");
        log << ' ' << type() << " breakpoint: pid=";
        log << thread->lwpid() << " owner=";
        log << this->thread()->lwpid() << " " << hex << addr() << dec << endl;

        if (sym)
        {
            log << ' ' << (void*)sym->addr() << ' ' << sym->name();
        }
        log << eventlog::endl;
    }
#endif // DEBUG
}


////////////////////////////////////////////////////////////////
//
// Invoked when an error occurs when trying to enable or disable a breakpoint.
//
void BreakPointBase::error(bool onOff, const char* errMsg) const volatile
{
    // One possible cause for this error to occur is when the
    // call is made from a different thread than the main thread.
    // This can be debugged by forcing an abort and examining
    // the stack trace.
    if (env::get_bool("ZERO_ABORT_ON_BREAKPOINT_ERROR", false))
    {
        abort();
    }

    cerr << "Could not " << (onOff ? "enable" : "disable") << ' ';
    cerr << type() << " breakpoint at 0x" << hex << addr() << dec;

    if (errMsg)
    {
        cerr << ": " << errMsg;
    }
    cerr << endl;
}


////////////////////////////////////////////////////////////////
size_t BreakPointBase::enum_actions(
    const char* name,
    EnumCallback<Action*>* callbackSink
    ) const volatile
{
    size_t result = 0;
    ActionList actions(const_cast<const ActionList&>(actions_));

    ActionList::const_iterator i = actions.begin();
    const ActionList::const_iterator end = actions.end();

    for (; i != end; ++i)
    {
        if (name == 0 || strcmp(name, (*i)->name()) == 0)
        {
            if (callbackSink)
            {
                callbackSink->notify(i->get());
            }
            ++result;
        }
    }
    return result;
}



////////////////////////////////////////////////////////////////
bool BreakPointBase::add_action(Action* action)
{
    if (action)
    {
        // use the cookie to ensure uniqueness of this action;
        // the search is linear, but I don't expect many actions
        // to be attached to the breakpoint.

        if (word_t cookie = action->cookie())
        {
            ActionList::const_iterator i = actions_.begin();
            for (; i != actions_.end(); ++i)
            {
              /*
                clog << __func__ << ": " << action->name() << ", "
                     << hex << (*i)->cookie()
                     << " new action=" << cookie << dec << endl; */

                if ((*i)->cookie() == cookie)
                {
                    return false;
                }
            }
        }
        actions_.push_back(RefPtr<Action>(action));

        if (Thread* thread = thread_.get())
        {
            if (Debugger* dbg = thread->debugger())
            {
                if (ZObject* context = dbg->current_action_context())
                {
                    actionContexts_[action] = context;
                }
            }
        }
        return true;
    }
    return false; // no action given, no action added
}


////////////////////////////////////////////////////////////////
ZObject* BreakPointBase::action_context(Action* action) const
{
    ActionMap::const_iterator i = actionContexts_.find(action);
    if (i != actionContexts_.end())
    {
        return i->second.get();
    }
    return NULL;
}



////////////////////////////////////////////////////////////////
size_t BreakPointBase::remove_actions(const char* name) volatile
{
    size_t resultCount = 0;

    ActionList& actions = const_cast<ActionList&>(actions_);
    ActionList::iterator i = actions.begin();

    while (i != actions.end())
    {
        if (!name || strcmp((*i)->name(), name) == 0)
        {
            i = actions.erase(i);
            ++resultCount;
        }
        else
        {
            ++i;
        }
    }
    return resultCount;
}


////////////////////////////////////////////////////////////////
size_t BreakPointBase::remove(Action* action, word_t cookie) volatile
{
    size_t result = 0;

    ActionList& actions = const_cast<ActionList&>(actions_);
    ActionList::iterator i = actions.begin();

    while (i != actions.end())
    {
        if ((i->get() == action)
         && (cookie == 0 || cookie == (*i)->cookie()))
        {
            i = actions.erase(i);
            ++result;
        }
        else
        {
            ++i;
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
size_t BreakPointBase::action_count() const volatile
{
    return const_cast<const ActionList&>(actions_).size();
}


////////////////////////////////////////////////////////////////
const BreakPointAction* BreakPointBase::action(size_t n) const volatile
{
    const BreakPointBase* THIS =
        const_cast<const BreakPointBase*>(this);
    if (n >= THIS->actions_.size())
    {
        throw out_of_range("action index out of range");
    }
    return THIS->actions_[n].get();
}


////////////////////////////////////////////////////////////////
static bool inline is_action_enabled(const RefPtr<BreakPointAction>& action)
{
    if (Switchable* sw = interface_cast<Switchable*>(action.get()))
    {
        return sw->is_enabled();
    }
    return true;
}



////////////////////////////////////////////////////////////////
void BreakPointBase::execute_actions(Thread* thread)
{
    // The list of actions associated with this breakpoint
    // may change during the execution of actions
    ActionList tmp(actions_);

    ActionList::iterator i = tmp.begin();
    for (size_t d = 0; i != tmp.end(); ++d)
    {
        assert(*i);

        if (!is_action_enabled(*i))
        {
            ++i;
            continue;
        }
        AutoContext autoContext(thread, action_context(i->get()));
        if ((*i)->execute(thread, this))
        {
            ++i;
        }
        else
        {
            // remove it from the master list
            ActionList::iterator j = actions_.begin();
            advance(j, d);

            actionContexts_.erase(j->get());
            actions_.erase(j);

            // remove it from tmp as well so that
            // destruction is not delayed --
            // dtor may have side effects
            i = tmp.erase(i);
        }
    }
}


////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& outs, BreakPoint::Type type)
{
    switch (type)
    {
    case BreakPoint::HARDWARE: outs << "hard"; break;
    case BreakPoint::SOFTWARE: outs << "soft"; break;
    case BreakPoint::EMULATED: outs << "emul"; break;

    default: assert(false); break;
    }
    return outs;
}


////////////////////////////////////////////////////////////////
void BreakPointBase::print(ostream& outs) const volatile
{
    const addr_t addr = this->addr();

    StateSaver<ios, ios::fmtflags> ifs(outs);

    const ActionList& actions =
        const_cast<const ActionList&>(actions_);

    ActionList::const_iterator i = actions.begin();
    const ActionList::const_iterator end = actions.end();

    for (; i != end; ++i)
    {
        outs.unsetf(ios::left);
        outs.setf(ios::right);

        const Type type = this->type();
        outs << '[' << setw(8) << setfill(' ');
        if (type == GLOBAL)
        {
            outs << "  GLOBAL] ";
        }
        else
        {
            outs << thread()->lwpid() << "] ";
        }
        outs << (enable_ > 0 ? "ON  " : "off ");
        outs << type << " 0x";
        outs << hex << setfill('0');
        outs << setw(2 * sizeof(addr_t)) << addr << dec << ' ';

        // print the name of the function at breakpoint

        SymbolMap* syms = thread()->symbols();
        if (syms)
        {
            outs.unsetf(ios::right);
            outs.setf(ios::left);

            outs << setfill(' ');
            size_t width = 24;

            outs << setw(width) << (*i)->name();

            RefPtr<Symbol> symbol(syms->lookup_symbol(addr));
            if (symbol)
            {
                outs << symbol->demangled_name(0)->c_str();
            }
        }
        outs << endl;
    }
}


////////////////////////////////////////////////////////////////
SoftwareBreakPoint::SoftwareBreakPoint
(
    BreakPointManagerBase* mgr,
    RefPtr<Thread>  thread,
    addr_t          addr
)
  : BreakPointBase(thread, addr)
  , mgr_(mgr)
{
}


////////////////////////////////////////////////////////////////
SoftwareBreakPoint::SoftwareBreakPoint
(
    const SoftwareBreakPoint& other,
    BreakPointManagerBase* mgr,
    Thread& thread
)
  : BreakPointBase(other, thread)
  , mgr_(mgr)
{
}


////////////////////////////////////////////////////////////////
SoftwareBreakPoint::~SoftwareBreakPoint() throw()
{
    try
    {
        disable();
    }
    catch (const std::exception& e)
    {
        clog << e.what() << " in ~SoftwareBreakpoint calling disable()\n";
    }
    // not catching (...) because all the exceptions thrown
    // should inherit off std::exception -- if not, then let
    // the whole thing terminate()
}


////////////////////////////////////////////////////////////////
static void debug_clone(BreakPointBase& bpnt)
{
#ifdef DEBUG
    if (Debugger* debugger = bpnt.thread()->debugger())
    {
        DebugChannel channel(__func__, debugger->verbose());
        eventlog::Stream<DebugChannel> log(channel, 0);
        log << "Cloned: " << bpnt << endl;
    }
#endif
}


////////////////////////////////////////////////////////////////
BreakPointBase*
SoftwareBreakPoint::clone(BreakPointManagerBase* base,
                          RefPtr<Thread> thread)
{
    assert(thread);

    BreakPointBase* bpnt = NULL;

    if (thread)
    {
        bpnt = new SoftwareBreakPoint(*this, base, *thread);
        debug_clone(*bpnt);
    }
    return bpnt;
}


namespace
{
    /**
     * Helper used by SoftwareBreakPoint::do_enable
     *
     * Implementation of the EnumCallback<Thread> interface that:
     * 1) ensures all threads are out of the "dangerous" area
     * 2) picks a valid thread (i.e. alive and attached)
     */
    CLASS EnumThreadCallback : public EnumCallback<Thread*>
    {
    public:
        EnumThreadCallback(Thread* thread, addr_t addr)
            : thread_(thread)
            , addr_(addr)
        { }

        virtual ~EnumThreadCallback() throw() {}

        RefPtr<Thread> thread() const { return thread_; }

    protected:
        void notify(Thread* thread)
        {
            assert(thread);

            if (!thread_)
            {
                thread_ = thread;
                return;
            }
            if ((thread == thread_.get()) ||
                (thread->process() != CHKPTR(thread_)->process()))
            {
                return;
            }
            if (thread_finished(*CHKPTR(thread_)) &&
               !thread_finished(*thread))
            {
                if (Target* target = thread->target())
                {
                    target->step_until_safe(*thread, addr_);
                }
                //
                // still good after stepping?
                //
                if (!thread_finished(*thread))
                {
                #ifdef DEBUG
                    clog << "resetting thread to " << thread->lwpid() << endl;
                #endif
                    thread_ = thread;
                }
            }
        }

    private:
        RefPtr<Thread>  thread_;
        addr_t          addr_;
    };
}


////////////////////////////////////////////////////////////////
bool SoftwareBreakPoint::do_enable(bool onOff) volatile
{
    bool success = true; // returned value

    debug_enable(onOff);

    // deref-ing should never fail here: we own a RefPtr<Thread>
    Thread& thr = *CHKPTR(thread());

    if (thread_finished(thr))
    {
        EnumThreadCallback threadsEnum(&thr, addr());
        if (Target* target = thr.target())
        {
            target->enum_threads(&threadsEnum);
        }
        reparent(threadsEnum.thread());
    }

    if (RefPtr<BreakPointManagerBase> mgr = mgr_.ref_ptr())
    {
        success = mgr->enable_software_breakpoint(thr, addr(), onOff);
    }
    else
    {
        success = false;
    }
    return success;
}


////////////////////////////////////////////////////////////////
bool SoftwareBreakPoint::is_enabled() const volatile
{
    bool isEnabled = BreakPointBase::is_enabled();
    return isEnabled;
}


////////////////////////////////////////////////////////////////
HardwareBreakPoint::HardwareBreakPoint
(
    RefPtr<Thread>  thread,
    addr_t          addr,
    int             debugRegister,
    bool            global,
    Condition       cond,
    Length          len
)
  : BreakPointBase(thread, addr, 1)
  , register_(debugRegister)
  , global_(global)
  , cond_(cond)
  , len_(len)
{
}


////////////////////////////////////////////////////////////////
HardwareBreakPoint::HardwareBreakPoint
(
    bool            global,
    RefPtr<Thread>  thread,
    addr_t          addr,
    int             debugRegister,
    Condition       cond
)
  : BreakPointBase(thread, RefPtr<Symbol>(), addr)
  , register_(debugRegister)
  , global_(global)
  , cond_(cond)
  , len_(DebugRegs::BREAK_ONE_BYTE)
{
}


////////////////////////////////////////////////////////////////
HardwareBreakPoint::~HardwareBreakPoint() throw()
{
    bool succeeded = false;

    try
    {
        assert(thread());

        if (thread_is_attached(*thread()))
        {
            assert(thread()->debug_regs());
            thread()->debug_regs()->clear(debug_register());
            succeeded = true;
        }
    }
    catch (const exception& e)
    {
        cerr << "Error clearing hardware breakpoint: ";
        cerr << e.what() << endl;
    }

    if (!succeeded && global_)
    {
        /* The attempt to clear the debug register may fail
           if the thread is not around anymore. If it is a
           local (per-thread) breakpoint, the error does not
           matter; for global breakpoints, try again with the
           default thread. */

        Thread* tp = NULL;
        if (Process* proc = thread()->process())
        {
            tp = proc->get_thread(DEFAULT_THREAD);
        }
        if (tp && tp != thread())
        {
            try
            {
                tp->debug_regs()->clear(debug_register());
                cerr << "Recovered." << endl;
            }
            catch (const exception& e)
            {
                cerr << "Error clearing global hardware breakpoint: ";
                cerr << e.what() << endl;
            }
        }
    }
}


////////////////////////////////////////////////////////////////
void HardwareBreakPoint::execute_actions(Thread* th)
{
    assert(th);
    assert(th->lwpid() == thread()->lwpid());

    BreakPointBase::execute_actions(th);
}


////////////////////////////////////////////////////////////////
bool HardwareBreakPoint::do_enable(Thread& thread,
                                   bool onOff,
                                   string& err) volatile throw()
{
    try
    {
        debug_enable(onOff, &thread);
        DebugRegs* dregs = CHKPTR(thread.debug_regs());

        if (onOff)
        {
            assert(dregs->addr_in_reg(register_) == addr());
        }

        dregs->enable(register_, onOff, global_, cond_, len_);
        return true;
    }
    catch (const exception& e)
    {
        err = e.what();
    }
    return false;
}


////////////////////////////////////////////////////////////////
bool HardwareBreakPoint::do_enable(bool onOff) volatile
{
    string err;

    Thread* thread = this->thread();

    while (thread && !do_enable(*thread, onOff, err))
    {
        error(onOff, err.c_str());

        if (!global_)
        {
            return false;
        }
        else if (Process* proc = thread->process())
        {
            // if the breakpoint is global, retry with main thread

            Thread* mainThread = proc->get_thread(DEFAULT_THREAD);

            if (mainThread != thread)
            {
                thread = mainThread;
                continue;
            }
        }
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////////
BreakPointBase*
HardwareBreakPoint::clone(BreakPointManagerBase* mgr,
                          RefPtr<Thread> thread)
{
    assert(mgr);
    assert(thread);

    BreakPointBase* bpnt = NULL;
    if (thread)
    {
        // to simplify things (what if there are not enough debug
        // breakpoints available, etc) just make an emulated breakpoint
        bpnt = new EmulatedBreakPoint(mgr, thread, addr());
        bpnt->append(this->actions());

        if (is_enabled() && !bpnt->is_enabled())
        {
            bpnt->enable();
        }
        debug_clone(*bpnt);
    }
    return bpnt;
}


////////////////////////////////////////////////////////////////
EmulatedBreakPoint::EmulatedBreakPoint
(
    BreakPointManagerBase* mgr,
    RefPtr<Thread> thread,
    addr_t addr
)
  : SoftwareBreakPoint(mgr, thread, addr)
{
}


////////////////////////////////////////////////////////////////
void EmulatedBreakPoint::execute_actions(Thread* t)
{
    assert(t);

    if ((t->lwpid() == thread()->lwpid())
     /* todo: add a Thread::is_equal() method to deal*/
     /* with OS-es where the lwp <--> thread mapping */
     /* is not one-to-one                            */
     /* || (t->thread_id() == thread()->thread_id()) */)
    {
        SoftwareBreakPoint::execute_actions(t);
    }
#if 0 // DEBUG
    else
    {
        clog << __func__ << ": " << t->lwpid();
        clog << " does not match " << thread()->lwpid() << endl;
    }
#endif
}


////////////////////////////////////////////////////////////////
BreakPointBase*
EmulatedBreakPoint::clone(BreakPointManagerBase* mgr,
                          RefPtr<Thread> thread)
{
    assert(thread);

    BreakPointBase* bpnt = NULL;
    if (thread)
    {
        bpnt = new EmulatedBreakPoint(*this, mgr, *thread);
        debug_clone(*bpnt);
    }
    return bpnt;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
