//
// $Id: on_call_return.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include <signal.h>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include "dharma/environ.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "zdk/types.h"
#include "zdk/type_system_util.h"
#include "zdk/type_tags.h"
#include "call_setup.h"
#include "context.h"
#include "debug_out.h"
#include "engine/ret_symbol.h"
#include "on_call_return.h"
#include "variant_impl.h"


using namespace std;

#ifdef DEBUG
 #define TRACE_RESULT_SYMBOL(f,s)                           \
    assert((s)->value());                                   \
    if (Interp::debug_enabled())                            \
    {                                                       \
        clog << __FILE__ << ':' << __LINE__ << ": ";        \
        clog << (f)->name() << '=' << (s)->type()->name();  \
        clog << ":" << (s)->value() << endl;                \
    }
#else
 #define TRACE_RESULT_SYMBOL(f,s)
#endif


////////////////////////////////////////////////////////////////
OnCallReturn::OnCallReturn
(
    RefPtr<Expr> expr,
    RefPtr<Symbol> func,
    const CallSetup& setup,
    Thread& thread,
    DebugInfoReader* debugInfo
)
  : CPUStateSaver(thread)
  , expr_(expr)
  , func_(func)
  , debugInfo_(debugInfo)
  , retAddr_(setup.ret_addr())
  , retType_(setup.ret_type())
  , frame_(0)
  , pc_(thread.program_count())
{
    if (!expr_)
    {
        throw invalid_argument("null expression in call return");
    }
    interp_ = expr_->interp();

    if (!interp_)
    {
        throw invalid_argument("null interpreter in call return");
    }
    if (!func_)
    {
        throw invalid_argument("null function in call return");
    }
    assert(!retType_.ref_ptr() == !retAddr_);

    // memorize the index of the stack frame in view
    if (StackTrace* stk = thread.stack_trace())
    {
        if (const Frame* frame = stk->selection())
        {
            frame_ = frame->index();
        }
    }
    oldAct_ = thread.debugger()->get_sig_action(SIGSEGV);

    // Clear the current signal, so that we do not deliver it
    // yet to the debugged program
    thread.set_signal(0);
}


////////////////////////////////////////////////////////////////
OnCallReturn::~OnCallReturn() throw()
{
    // not a good idea: it breaks test_arg.sh
    //thread()->debugger()->set_sig_action(SIGSEGV, oldAct_.get());
}


////////////////////////////////////////////////////////////////
const char* OnCallReturn::name() const
{
    return "ON_CALL_RET";
}


////////////////////////////////////////////////////////////////
word_t OnCallReturn::cookie() const
{
    return 0;
}


////////////////////////////////////////////////////////////////
static void restore_selected_frame(Thread* thread, size_t frame)
{
    if (StackTrace* stackTrace = CHKPTR(thread)->stack_trace())
    {
        stackTrace->select_frame(frame);
    }
}


////////////////////////////////////////////////////////////////
static void schedule_interactive_mode(Thread& thread)
{
    if (Debugger* dbg = thread.debugger())
    {
        dbg->schedule_interactive_mode(&thread, E_EVAL_COMPLETE);
        thread_set_event_description(thread,
            "Expression evaluation complete");
    }
}


////////////////////////////////////////////////////////////////
RefPtr<DebugSymbol> OnCallReturn::ret_symbol(Thread* thread)
{
    RefPtr<DebugSymbol> sym;

    if (debugInfo_)
    {
        sym = debugInfo_->get_return_symbol(thread, func_.get());
    }
    else
    {
        sym = ::ret_symbol(thread, func_);
    }
    return sym;
}


////////////////////////////////////////////////////////////////
bool
OnCallReturn::compute_ret_value(Thread* thread,
                                RefPtr<DebugSymbol> dsym)
{
    assert(dsym->type());

    if (!dsym->value())
    {
        dsym->read(NULL);
    }
    if (dsym->value())
    {
        TRACE_RESULT_SYMBOL(func_, dsym);

        if (retAddr_)
        {
            if (static_cast<word_t>(retAddr_) != thread->result())
            {
                CHKPTR(interp_)->error("small object returned in registers");
                return false;
            }

            // Hack for returning objects by value:
            // make the symbol's value a string representation
            // of its address
            ostringstream buf;
            buf << hex << showbase << retAddr_;

            RefPtr<SharedString> val = shared_string(buf.str());
            dsym = dsym->clone(val.get(), CHKPTR(retType_.ref_ptr().get()));

            dsym->read(this);
            DEBUG_OUT << dsym->type()->name() << endl;
        }
        else
        {
            // Cloning the symbol with a specified (non-null) value
            // has the side-effect of marking the clone as constant;
            // this is useful for values returned in CPU registers,
            // because it disables re-reading the said registers which
            // may have changed in the meanwhile.

            dsym = dsym->clone(dsym->value(), dsym->type());
            dsym->read(this);
        }
        TRACE_RESULT_SYMBOL(func_, dsym);
    }
    // set the result inside the pending expression
    RefPtr<Variant> var = new VariantImpl(*dsym);
    expr_->set_result(var);

    return true;
}


////////////////////////////////////////////////////////////////
bool OnCallReturn::execute(Thread* thread, BreakPoint* bpnt)
{
    if (CHKPTR(thread) != this->thread())
    {
        // the event occurred on a different thread, don't discard
        return true;
    }

    // invoked as signal action upon a SEGV?
    if (!bpnt
        && (thread->signal() == SIGSEGV)
        && (thread->program_count() != INVALID_PC_ADDR))
    {
        DEBUG_OUT << "genuine SIGSEGV\n";

        if (oldAct_.get())
        {
            DEBUG_OUT << "invoking old action\n";
            oldAct_->execute(thread, bpnt);
        }
        return true; // keep the action
    }
    bool haveRetValue = false;
    if (RefPtr<DebugSymbol> dsym = ret_symbol(thread))
    {
        haveRetValue = compute_ret_value(thread, dsym);
    }
    else
    {
        DEBUG_OUT << "null return symbol\n";
    }
    if (!haveRetValue)
    {
        word_t ret = thread->result();
        RefPtr<VariantImpl> var = new VariantImpl;
        if (thread->is_32_bit())
        {
            put(var.get(), int(ret));
        }
        else
        {
            put(var.get(), ret);
        }
        TypeSystem& typesys = interp_->context().type_system();
        expr_->set_result(var);
        expr_->set_type(GET_INT_TYPE(typesys, long));
    }

    // This return action may be programmed to fire on a
    // breakpoint, or upon a signal; when invoking a function
    // inside of the debuggee, we can either set a breakpoint
    // at the return address of the function, or push an invalid
    // program counter on the stack. The latter seems to work
    // better with recursive functions.
    addr_t addr = bpnt ? bpnt->addr() : thread->program_count();
    if (!bpnt && thread->signal() == SIGSEGV)
    {
        if (addr == INVALID_PC_ADDR)
        {
            thread->set_signal(0);
            // restore old action
            if (Debugger* debugger = thread->debugger())
            {
                debugger->set_sig_action(SIGSEGV, oldAct_.get());
            }
        }
        else
        {
            return true;
        }
    }
    bool interactive = true;
    interface_cast<Runnable&>(*thread).set_program_count(pc_);

    // the interpreter must resume in the same stack context
    restore_selected_frame(thread, frame_);
    // addr = thread->stack_trace()->selection()->program_count();

    int result = interp_->resume(addr, this, &interactive);
    DEBUG_OUT << "interactive=" << boolalpha << interactive << endl;

    // EVAL_DONE or EVAL_ERROR? might need to enter interactive mode
    if (result != Interp::EVAL_AGAIN)
    {
        restore_state();
        restore_selected_frame(thread, frame_);

        if (interactive)
        {
            schedule_interactive_mode(*thread);
        }
    }
    return false; // discard this action after executing
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
