//
// $Id$
//
// Classes: FrameImpl, StackTraceImpl
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/stdexcept.h"
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/frame_handler.h"
#include "zdk/thread_util.h"
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "generic/temporary.h"
#include "debugger_engine.h"
#include "stack_trace.h"
#include <iostream>


using namespace std;

static Symbol* const no_symbol = (Symbol*)-1;


FrameImpl::FrameImpl(StackTraceImpl& trace, const Thread& t)
    : trace_(&trace)
    , pc_(t.program_count())
    , sp_(t.stack_pointer())
    , fp_(t.frame_pointer())
    , realPC_(t.program_count())
    , index_(0)
    , sigHandler_(false)
    , skip_(false)
{
}


FrameImpl::FrameImpl(/* const */ FrameImpl& other, size_t index)
    : trace_(other.trace_)
    , pc_(other.pc_)
    , sp_(other.sp_)
    , fp_(other.fp_)
    , realPC_(other.realPC_)
    , index_(index)
  // intentional:
  //, sigHandler_(other.sigHandler_)
    , sigHandler_(false)
    , skip_(false)
    , userObj_(other.userObj_)
{
    other.userObj_.clear();
}


FrameImpl::~FrameImpl() throw()
{
}


addr_t FrameImpl::program_count() const
{
    return pc_;
}


addr_t FrameImpl::stack_pointer() const
{
    return sp_;
}


addr_t FrameImpl::frame_pointer() const
{
    return fp_;
}


void FrameImpl::set_program_count(addr_t pc)
{
    if (pc_ != pc)
    {
        sym_.reset();
    }
    pc_ = pc;

    if (!realPC_)
    {
        realPC_ = pc;
    }
}


Symbol* FrameImpl::function(Symbol* fun) const
{
    if (!sym_)
    {
        if (RefPtr<StackTraceImpl> trace = trace_.ref_ptr())
        {
            RefPtr<Symbol> sym;
            if (fun)
            {
                if (fun != no_symbol)
                {
                    sym = fun;
                }
            }
            else if (RefPtr<Thread> thread = trace_->thread())
            {
                if (SymbolMap* symbols = thread->symbols())
                {
                    sym = symbols->lookup_symbol(pc_);
                }
            }
            sym_ = sym;
        }
    }
    return sym_.get();
}


void FrameImpl::set_user_object(const char* key, ZObject* object)
{
    assert(key);
    RefPtr<ZObject> objPtr(object);
    ObjectMap::iterator i = userObj_.find(key);
    if (i == userObj_.end())
    {
        userObj_.insert( make_pair(key, objPtr));
    }
    else
    {
        i->second = objPtr;
    }
}


ZObject* FrameImpl::get_user_object(const char* key) const
{
    assert(key);
    ObjectMap::const_iterator i = userObj_.find(key);
    return (i == userObj_.end()) ? NULL : i->second.get();
}


StackTraceImpl::StackTraceImpl(const Thread& thread)
    : thread_(&thread)
    , complete_(false)
    , unwinding_(false)
    , sel_(0)
    , frameHandlerCount_(0)
{
}


StackTraceImpl::~StackTraceImpl() throw()
{
}


size_t StackTraceImpl::size() const
{
    return frames_.size();
}


Frame* StackTraceImpl::frame(size_t index) const
{
    if (index >= frames_.size())
    {
    #if DEBUG
        clog << "index=" << index << ", stack depth=";
        clog << frames_.size() << endl;
    #endif
        if (env::get("ZERO_ABORT_ON_ERROR", 0) == 2)
        {
            abort();
        }
        throw std::out_of_range("Frame index is out of range");
    }
    return frames_[index].get();
}


void StackTraceImpl::select_frame(size_t index)
{
    if (index >= frames_.size())
    {
    #ifdef DEBUG
        clog << "index=" << index << ", stack depth=";
        clog << frames_.size() << endl;
    #endif
        if (env::get("ZERO_ABORT_ON_ERROR", 0) == 2)
        {
            abort();
        }
        throw std::out_of_range("Frame index is out of range");
    }
    sel_ = index;
}


Frame* StackTraceImpl::selection() const
{
    return frame(sel_);
}


static size_t stack_max_depth()
{
    static size_t maxDepth = env::get("ZERO_MAX_STACK", 1024);
    return maxDepth;
}


void stack_check_max_depth(size_t& depth)
{
    if (depth > stack_max_depth())
    {
        depth = stack_max_depth();
    }
}


void StackTraceImpl::unwind(const Thread& thread, size_t depth, const StackTrace* old)
{
    if (unwinding_)
    {
        // prevent stack overflow: if we attempt to read a register
        // from a saved frame while unwinding is in progress, it may
        // cause this function to be re-enterred
        return;
    }
    Temporary<bool> setFlag(unwinding_, true);

    const bool useFrameHandlers = use_frame_handlers();

    if (useFrameHandlers)
    {
        if (DebuggerEngine* eng = interface_cast<DebuggerEngine*>(thread.debugger()))
        {
            eng->init_frame_handlers(thread);
        }
    }
    stack_check_max_depth(depth);
    frames_.reserve(128);

    RefPtr<FrameImpl> f(new FrameImpl(*this, thread));

    if (!unwind_begin(thread, *f, depth) || f->program_count() == 0)
    {
        complete_ = true;
        return;
    }

    for (; frames_.size() < depth; ) // walk the stack
    {
        // try frame handlers first
        RefPtr<Frame> next;
        if (useFrameHandlers)
        {
            next = unwind_frame_handlers(thread, *f);
        }

        if (next)
        {
            f->set_program_count(next->program_count());
            f->set_stack_pointer(next->stack_pointer());
            f->set_frame_pointer(next->frame_pointer());

            assert(f->real_program_count() || f->program_count() == 0);

            f->set_real_program_count(next->real_program_count());
            ++frameHandlerCount_;
        }
        else
        {
            f->set_real_program_count(0);
            //
            // unwind_frame is implemented in an architecture-specific
            // way (see stack-386.cpp, stack-x86_64.cpp)
            //
            if (!unwind_frame(thread, *f))
            {
                complete_ = true;
                break;
            }
        }

        if (f->program_count() == 0)
        {
            complete_ = true;
            break;
        }
    #if 0
        //
        // DWARF support for frame unwinding renders this obsolete
        //
        if (verify_recover(thread, *f))
        {
            break;
        }
    #endif
        if (f->is_signal_handler() && !frames_.empty())
        {
            f->set_signal_handler(false);
            frames_.back()->set_signal_handler();
        }
        if (f->skip())
        {
            continue;
        }
        RefPtr<FrameImpl> fp(new FrameImpl(*f, frames_.size()));
        frames_.push_back(fp);
    }
    if (frames_.size() == stack_max_depth())
    {
        complete_ = true;
    }

    if (old)
    {
        // "double-buffering": minimize symbol lookups
        copy_symbols(*old);
    }
}


void StackTraceImpl::copy_symbols(const StackTrace& other)
{
    // todo: work it from the bottom, and break out of
    // the loop as soon as the frames don't match

    const size_t n = std::min(frames_.size(), other.size());

    for (size_t i = 0; i != n; ++i)
    {
        Frame* f = other.frame(i);
        Frame* g = frames_[i].get();

        if (f->program_count() != g->program_count())
        {
            continue;
        }

        if (Symbol* sym = f->function(no_symbol))
        {
            //clog << " ##" << i << ": " << sym << endl;
            g->function(sym);
        }
    }
}


/**
 * Try to unwind one step, using frame handlers, if any.
 * @return null pointer on failure, or the next frame
 */
RefPtr<Frame> StackTraceImpl::unwind_frame_handlers(const Thread& thread, Frame& current)
{
    RefPtr<Frame> result;
    try
    {
        if (use_frame_handlers())
        {
            const size_t sel = frames_.empty() ? sel_ : frames_.size() - 1;

            // When evaluating DWARF frame expressions, we may need to
            // get the program counter, stack pointer, etc, in the context
            // of the previous frame, so we need to make the previous
            // frame the current selection

            Temporary<size_t> selection(sel_, sel);

            Debugger* dbg = thread.debugger();

            if (DebuggerEngine* eng = interface_cast<DebuggerEngine*>(dbg))
            {
                result = eng->unwind_frame(thread, current);
            }
        }
    }
    catch (const exception& e)
    {
        clog << __func__ << ": " << e.what() << endl;
    }
    return result;
}



bool StackTraceImpl::use_frame_handlers()
{
#if defined(__x86_64__)
    static bool flag(env::get_bool("ZERO_USE_FRAME_HANDLERS", 1));
#else
    static bool flag(env::get_bool("ZERO_USE_FRAME_HANDLERS", 0));
#endif
    return flag;
}



StackTrace* StackTraceImpl::copy() const
{
    RefPtr<StackTraceImpl> dup;

    if (RefPtr<Thread> thread = thread_.lock())
    {
        dup = new StackTraceImpl(*thread);

        dup->complete_ = complete_;
        dup->frames_.assign(frames_.begin(), frames_.end());
        dup->sel_ = sel_;
    }
    return dup.detach();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
