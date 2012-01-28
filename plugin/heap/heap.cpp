//
// A debugger plugin for monitoring memory allocation.
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "zdk/config.h"
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <cassert>
#include <iostream>
#include "zdk/argv_util.h"
#include "zdk/runtime.h"
#include "zdk/thread_util.h"
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "block.h"
#include "cmd.h"
#include "heap.h"

using namespace std;

/* Properties stored inside the current thread object */
#define ALLOC_LEVEL ".alloc"        /* re-entrancy level    */
#define ALLOC_SIZE  ".alloc_size"   /* requested size       */
#define ALLOC_ADDR  ".alloc_addr"   /* requested addr       */
#define ALLOC_TRACE ".alloc_trace"

#ifdef DEBUG
 #define dbgout(level) if (debugLevel_ <= level); \
    else clog << "[heap] " << __func__ << ": "
#else
 #define dbgout(level) while(0) clog
#endif

static const size_t stackDepth = env::get("ZERO_HEAP_STACK", 4);


int32_t query_version(int32_t* minor)
{
    if (minor) *minor = ZDK_MINOR;
    return ZDK_MAJOR;
}


void query_plugin(InterfaceRegistry* registry)
{
    assert(registry);
    registry->update(DebuggerPlugin::_uuid());
}



Plugin* create_plugin(uuidref_t iid)
{
    Plugin* plugin = 0;

    if (uuid_equal(DebuggerPlugin::_uuid(), iid))
    {
        plugin = new HeapPlugin();
    }
    return plugin;
}


#if __i386__
static inline word_t fun_param_from_stack(Thread& thread, size_t n)
{
    return thread_peek_stack(thread, n);
}
#elif __x86_64__
static inline word_t fun_param_from_stack(Thread& thread, size_t n)
{
    switch (n)
    {
    case 1: return thread.read_register(RDI / 8, true);
    case 2: return thread.read_register(RSI / 8, true);
    case 3: return thread.read_register(RDX / 8, true);
    case 4: return thread.read_register(R8 / 8, true);
    case 5: return thread.read_register(R9 / 8, true);
    }
    return thread_peek_stack(thread, n);
}
#endif // __x86_64__


 void HeapPlugin::add_commands()
{
    assert(debugger_);

    CommandCenter& cmdCenter = interface_cast<CommandCenter&>(*debugger_);

    on_.reset(new HeapMonitorOn(this));
    off_.reset(new HeapMonitorOff(this));
    cmdCenter.add_command(on_.get());
    cmdCenter.add_command(off_.get());
    //cmdCenter.add_command(new HeapMonitorClear(this));
}


static inline void memorize_stack_trace(Thread* thread)
{
    assert(thread);

    if (StackTrace* stack = thread->stack_trace(stackDepth))
    {
        RefPtr<StackTrace> trace = stack->copy();

        if (trace)
        {
            const size_t n = trace->size();

            for (size_t i = 0; i != n; ++i)
            {
                if (Frame* frame = trace->frame(i))
                {
                    frame->function();
                }
            }
        }
        thread->set_user_object(ALLOC_TRACE, trace.get());
    }
}


/**
 * Helper function object; implements the BreakPoint::Action
 * interface. Executing the action translates into calling
 * a HeapPlugin method.
 */
class ZDK_LOCAL CallbackAction : public ZObjectImpl<BreakPoint::Action>
{
public:
    CallbackAction(HeapPlugin* heap, Callback cb, bool temp, word_t cookie = 0)
        : heap_(heap), cb_(cb), temporary_(temp), cookie_(cookie)
    { }

    /// BreakPoint::Action interface ///
    virtual const char* name() const { return "heapmon"; }

    virtual bool execute(Thread* thread, BreakPoint* bpnt)
    {
        (heap_->*cb_)(thread, bpnt);
        return !temporary_; // keep breakpoint action?
    }

    virtual word_t cookie() const { return cookie_; }

private:
    HeapPlugin* heap_;
    Callback    cb_;
    bool        temporary_; // remove after executing?
    word_t      cookie_;
};



HeapPlugin::HeapPlugin()
    : debugger_(0)
    , total_(0)
    , debugLevel_(0)
    , enabled_(false)
{
}



HeapPlugin::~HeapPlugin() throw()
{
}


void HeapPlugin::release()
{
    delete this;
}



bool HeapPlugin::initialize(Debugger* dbg, int* ac, char*** av)
{
    // pre-conditions:
    assert(dbg);
    assert(ac);
    assert(av);

    debugger_ = dbg;

    for (int i = 1; i < *ac; )
    {
        if (argv_match(ac, av, i, "--heap-debug"))
        {
            ++debugLevel_;
        }
        else if (argv_match(ac, av, i, "--heap-mon"))
        {
            enabled_ = true;
        }
        else
        {
            ++i;
        }
    }
    return true;
}


void HeapPlugin::start()
{
    if (debugger_)
    {
        add_commands();
    }
}


void HeapPlugin::shutdown()
{
}



void HeapPlugin::register_streamable_objects(ObjectFactory*)
{
}



void HeapPlugin::on_table_init(SymbolTable* symtab)
{
}


/**
 * Each time the debugger engine is done with reading a symbol
 * table, attempt to set breakpoints at the functions of interest
 */
void HeapPlugin::on_table_done(SymbolTable* symtab)
{
    if (symtab)
    {
        set_breakpoint(*symtab, "calloc", &HeapPlugin::on_calloc);
        set_breakpoint(*symtab, "free", &HeapPlugin::on_free);
        set_breakpoint(*symtab, "malloc", &HeapPlugin::on_malloc);
        set_breakpoint(*symtab, "realloc", &HeapPlugin::on_realloc);
    }
}



void HeapPlugin::on_attach(Thread*)
{
    enable(enabled_);
}



void HeapPlugin::on_detach(Thread* thread)
{
    if (thread == 0) // detached from all threads?
    {
        dbgout(0) << "clearing all counters\n";
        blocksByAddr_.clear();
        blocksBySize_.clear();
        total_ = 0;
    }
}



bool HeapPlugin::on_event(Thread*, EventType)
{
    return false; // not interested in events
}



void HeapPlugin::on_program_resumed()
{
}



void HeapPlugin::on_insert_breakpoint(volatile BreakPoint*)
{
}



void HeapPlugin::on_remove_breakpoint(volatile BreakPoint*)
{
}



bool HeapPlugin::on_progress(const char*, double, word_t)
{
    return true;
}


size_t HeapPlugin::enum_blocks(EnumCallback<const HeapBlock*>* cb, EnumOption option)
{
    size_t count = 0;

    switch (option)
    {
    case NONE:
        // enumerate blocks in increasing order of their addresses
        for (BlocksByAddr::const_iterator i = blocksByAddr_.begin();
             i != blocksByAddr_.end();
             ++i, ++count)
        {
             if (cb) cb->notify(i->second.get());
        }
        break;

    case SORT_SIZE_INCREASING:
        for (BlocksBySize::iterator i = blocksBySize_.begin();
            i != blocksBySize_.end();)
        {
            if (i->second->is_free())
            {
                blocksBySize_.erase(i++);
            }
            else
            {
                if (cb) cb->notify(i->second.get());
                ++i, ++count;
            }
        }
        break;

    case SORT_SIZE_DECREASING:
        for (BlocksBySize::reverse_iterator i = blocksBySize_.rbegin();
            i != blocksBySize_.rend();)
        {
            if (i->second->is_free())
            {
                BlocksBySize::iterator j = i.base();
                ++i;
                blocksBySize_.erase(j);
            }
            else
            {
                if (cb) cb->notify(i->second.get());
                ++i, ++count;
            }
        }
        break;
    }
    return count;
}


size_t HeapPlugin::total() const
{
    return total_;
}



void HeapPlugin::set_breakpoint(
    Thread&     thread,
    addr_t      addr,
    Callback    cb,
    bool        temp,
    word_t      cookie)
{
    assert(debugger_);

    if (BreakPointManager* bpntMgr = interface_cast<BreakPointManager*>(debugger_))
    {
        RefPtr<BreakPoint::Action> action(new CallbackAction(this, cb, temp, cookie));

        // NOTE: if a hardware breakpoint cannot be set, the engine
        // automatically falls back to a software emulated breakpoint
        BreakPoint::Type type = temp ? BreakPoint::HARDWARE : BreakPoint::SOFTWARE;
        bpntMgr->set_breakpoint(&interface_cast<Runnable&>(thread), type, addr, action.get());
    }
}



void HeapPlugin::set_breakpoint(
    SymbolTable&    symtab,
    const char*     symName,
    Callback        callback)
{
    assert(symName); // pre-condition

    RefPtr<Thread> thread = debugger_->get_thread(DEFAULT_THREAD);
    if (!thread.is_null())
    {
        SymbolEnum syms;
        SymbolTable::LookupMode mode = SymbolTable::LKUP_SYMBOL | SymbolTable::LKUP_DYNAMIC;
        symtab.enum_symbols(symName, &syms, mode);

        SymbolEnum::const_iterator i = syms.begin();
        for (; i != syms.end(); ++i)
        {
            set_breakpoint(*thread, (*i)->addr(), callback, false);
        }
    }
}



void HeapPlugin::on_calloc(Thread* thread, BreakPoint* bpnt)
{
    assert(thread);
    assert(bpnt);

    if (!enabled_ || !thread || !bpnt)
    {
        return;
    }
    dbgout(0) << reinterpret_cast<void*>(bpnt->addr()) << endl;

    word_t count = 0;

    thread->get_user_data(ALLOC_LEVEL, &count);

    if (count++ == 0)
    {
        // the function prototype for calloc is:
        // void *calloc(size_t nmemb, size_t size);
        // peek arguments from the stack:
        const size_t elem = fun_param_from_stack(*thread, 1);
        const size_t size = fun_param_from_stack(*thread, 2);

        dbgout(0) << "elem=" << elem << " size=" << size << endl;

        // memorize the requested size
        thread->set_user_data(ALLOC_SIZE, size * elem);

        memorize_stack_trace(thread);
    }

    const addr_t caller = thread_peek_stack(*thread);
    set_breakpoint(*thread,
                    caller,
                    &HeapPlugin::on_calloc_return,
                    true, // temporary
                    count);

    // calloc may internally use malloc -- we only want to track the
    // outermost calls
    thread->set_user_data(ALLOC_LEVEL, count);
}



void HeapPlugin::on_calloc_return(Thread* thread, BreakPoint* bpnt)
{
    assert(thread);
    assert(bpnt);

    dbgout(0) << reinterpret_cast<void*>(bpnt->addr()) << endl;
    on_malloc_return(thread, bpnt);
}



void HeapPlugin::on_malloc(Thread* thread, BreakPoint* bpnt)
{
    assert(thread);
    assert(bpnt);

    dbgout(0) << enabled_ << endl;
    if (!enabled_ || !thread || !bpnt)
    {
        return;
    }
    dbgout(0) << reinterpret_cast<void*>(bpnt->addr()) << endl;

    word_t count = 0;
    thread->get_user_data(ALLOC_LEVEL, &count);

    if (count++ == 0)
    {
        const size_t size = fun_param_from_stack(*thread, 1);

        thread->set_user_data(ALLOC_SIZE, size);
        memorize_stack_trace(thread);
    }
    dbgout(0) << "count=" << count << endl;

    // caller's address is the return addr on the stack
    const addr_t caller = thread_peek_stack(*thread);

    dbgout(0) << "caller=" << (void*)caller << endl;

    set_breakpoint(*thread,
                    caller,
                    &HeapPlugin::on_malloc_return,
                    true /* temporary */,
                    count);
    thread->set_user_data(ALLOC_LEVEL, count);
}



/**
 * Upon return, memorize the allocated block (if malloc was successful)
 */
void HeapPlugin::on_malloc_return(Thread* thread, BreakPoint* bpnt)
{
    assert(thread);
    assert(bpnt);

    dbgout(0) << reinterpret_cast<void*>(bpnt->addr()) << endl;

    word_t count = 0;

    thread->get_user_data(ALLOC_LEVEL, &count);
    dbgout(0) << "count=" << count << endl;

    if (count == 0)
    {
        cerr << __func__ << ": mismatched call level, non-zero expected.\n";
        return;
    }
    thread->set_user_data(ALLOC_LEVEL, --count);

    if (count == 0)
    {
        word_t size = 0;
        thread->get_user_data(ALLOC_SIZE, &size);

        const addr_t addr = thread->result();

        dbgout(0) << "size=" << size << " result=" << (void*)addr << endl;

        // if malloc succeeded, then memorize the address of the
        // allocated block, its size, and the call stack
        if (addr != static_cast<addr_t>(-1))
        {
            add_heap_block(*thread, addr, size);
        }
    }
}



void HeapPlugin::on_realloc(Thread* thread, BreakPoint* bpnt)
{
    assert(thread);
    assert(bpnt);

    if (!enabled_ || !thread || !bpnt)
    {
        return;
    }
    dbgout(0) << reinterpret_cast<void*>(bpnt->addr()) << endl;

    word_t count = 0;

    thread->get_user_data(ALLOC_LEVEL, &count);

    const addr_t caller = thread_peek_stack(*thread);

    // prototype is: void *realloc(void *ptr, size_t size);
    // peek args from the stack:
    const addr_t addr = fun_param_from_stack(*thread, 1);
    const word_t size = fun_param_from_stack(*thread, 2);

    thread->set_user_data(ALLOC_ADDR, addr);
    thread->set_user_data(ALLOC_SIZE, size);
    memorize_stack_trace(thread);

    Callback callback = NULL;
    if (addr == 0)
    {
        // realloc behaves like malloc()
        callback = &HeapPlugin::on_malloc_return;
    }
    else if (size == 0)
    {
        // realloc behaves like free()
        callback = &HeapPlugin::on_free_return;
    }
    else
    {
        callback = &HeapPlugin::on_realloc_return;
    }
    assert(callback);
    set_breakpoint(*thread, caller, callback, true);

    thread->set_user_data(ALLOC_LEVEL, ++count);
}



void HeapPlugin::on_realloc_return(Thread* thread, BreakPoint* bpnt)
{
    assert(thread);
    assert(bpnt);

    dbgout(0) << reinterpret_cast<void*>(bpnt->addr()) << endl;

    word_t count = 0;

    thread->get_user_data(ALLOC_LEVEL, &count);

    assert(count > 0);
    if (count == 0)
    {
        cerr << __func__ << ": mismatched call level, non-zero expected.\n";
        return;
    }
    thread->set_user_data(ALLOC_LEVEL, --count);

    if (count == 0)
    {
        word_t addr = 0;
        word_t size = 0;

        thread->get_user_data(ALLOC_ADDR, &addr);
        thread->get_user_data(ALLOC_SIZE, &size);

        // the result of realloc()
        if (addr_t ptr = thread->result())
        {
            remove_heap_block(addr);

            add_heap_block(*thread, ptr, size);
        }
    }
}



/**
 * Memorize the address to be free-d. Set a temp breakpoint inside the
 * caller of free, so that we remove the block from our internal maps
 * as soon as free() returns.
 */
void HeapPlugin::on_free(Thread* thread, BreakPoint* bpnt)
{
    assert(thread);
    assert(bpnt);

    dbgout(0) << reinterpret_cast<void*>(bpnt->addr()) << endl;

    word_t count = 0;

    thread->get_user_data(ALLOC_LEVEL, &count);

    // pointer to be free-d is first (and only) arg on the stack
    const addr_t addr = fun_param_from_stack(*thread, 1);

    dbgout(0) << "ptr=" << reinterpret_cast<void*>(addr) << endl;

    const addr_t caller = thread_peek_stack(*thread);
    set_breakpoint(*thread,
                    caller,
                    &HeapPlugin::on_free_return,
                    true);

    thread->set_user_data(ALLOC_ADDR, addr);
    thread->set_user_data(ALLOC_LEVEL, ++count);
}



void HeapPlugin::on_free_return(Thread* thread, BreakPoint* bpnt)
{
    assert(thread);
    assert(bpnt);

    dbgout(0) << reinterpret_cast<void*>(bpnt->addr()) << endl;

    word_t count = 0;

    thread->get_user_data(ALLOC_LEVEL, &count);

    if (count == 0)
    {
        cerr << __func__ << ": mismatched call level, non-zero expected.\n";
        return;
    }
    thread->set_user_data(ALLOC_LEVEL, --count);

    if (count == 0)
    {
        word_t addr = 0;

        thread->get_user_data(ALLOC_ADDR, &addr);

        if (addr)
        {
            remove_heap_block(addr);
        }
    }
}



void HeapPlugin::add_heap_block(Thread& thread,
                                addr_t  addr,
                                size_t  size)
{
    RefPtr<StackTrace> trace =
        interface_cast<StackTrace*>(thread.get_user_object(ALLOC_TRACE));

    if (trace.is_null())
    {
        dbgout(0) << "no trace\n";
        return;
    }
    thread.set_user_object(ALLOC_TRACE, NULL);

    RefPtr<HeapBlockImpl> blk = new HeapBlockImpl(addr, size, trace);

    blocksByAddr_[addr] = blk;
    blocksBySize_.insert(make_pair(size, blk));

    total_ += size;
    dbgout(0) << "total=" << total_ << endl;
}



void HeapPlugin::remove_heap_block(addr_t addr)
{
    BlocksByAddr::iterator i = blocksByAddr_.find(addr);

    if (i != blocksByAddr_.end())
    {
        i->second->mark_as_free();

        assert(total_ >= i->second->size());
        total_ -= i->second->size();

        blocksByAddr_.erase(i);
    }
}


void HeapPlugin::enable(bool flag)
{
    enabled_ = flag;

    CommandCenter* cmdCenter = interface_cast<CommandCenter*>(debugger_);

    cmdCenter->enable_command(on_.get(), !enabled_);
    cmdCenter->enable_command(off_.get(), enabled_);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
