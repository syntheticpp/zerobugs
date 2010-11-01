//
// $Id: watchpoint.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <sstream>
#include "zdk/check_ptr.h"
#include "zdk/data_type.h"
#include "zdk/thread_util.h"
#include "debugger_engine.h"
#include "watchpoint.h"

using namespace std;


WatchPoint::WatchPoint
(
    RefPtr<Thread>  thread,
    WatchType       type,
    addr_t          addr,
    int             reg,
    bool            global,
    Condition       cond
)
  : HardwareBreakPoint(global, thread, addr, reg, cond)
  , type_(type)
  , scope_(0)
{
}


WatchPoint::~WatchPoint() throw()
{
}


bool WatchPoint::in_scope(Thread* thread) const
{
    if (scope_ == 0 || thread == 0) // global scope?
    {
        return true;
    }
    StackTrace* stackTrace = CHKPTR(thread->stack_trace());
    if (!stackTrace || stackTrace->size() == 0)
    {
        return false;
    }

    bool result = false;

    // walk the stack starting at selected frame
    if (Frame* f = stackTrace->selection())
    {
#ifdef DEBUG
        clog << __func__ << ": scope=" << hex << scope_ << dec;
        clog << "; inspecting stack from #" << f->index() << endl;
#endif
        const size_t stackSize = stackTrace->size();
        for (size_t i = f->index(); i != stackSize;)
        {
            if (!f->function())
            {
                ++i;
                continue;
            }
            addr_t addr = f->function()->addr();
            addr -= f->function()->offset();
            result = (addr == scope_);
    #ifdef DEBUG
            clog << __func__ << "=" << result;
            clog << " addr=" << hex << addr << dec << endl;
    #endif
            if ((result) || (++i == stackSize))
            {
                break;
            }
            if ((f = stackTrace->frame(i)) == NULL)
            {
                break;
            }
        }
    }
    return result;
}


MemoryWatch::MemoryWatch()
    : SwitchableAction("memory watch", true, 0)
    , watchPoint_(0)
{
}


MemoryWatch::~MemoryWatch() throw()
{
}


bool MemoryWatch::in_scope(Thread* thread) const
{
    return CHKPTR(watchPoint_)->in_scope(thread);
}


void MemoryWatch::execute_impl(Debugger& engine,
                               Thread* thread,
                               BreakPoint*)
{
    assert(thread);
    if (in_scope(thread))
    {
        engine.schedule_interactive_mode(thread, E_THREAD_BREAK);
    }
}


const char* MemoryWatch::description() const
{
    if (description_.empty())
    {
        std::ostringstream s;
        assert(watchPoint_);
        if (watchPoint_->is_global())
        {
            s << "all threads";
        }
        else
        {
            s << "thread=" << watchPoint_->thread()->lwpid();
        }

        const_cast<std::string&>(description_) =
            s.str() + ", " + description_impl();
    }
    return description_.c_str();
}


std::string MemoryWatch::description_impl() const
{
    std::string s("break on ");

    assert(watchPoint_);
    if (watchPoint_->watch_type() == WATCH_WRITE)
    {
        s += "write";
    }
    else
    {
        s += "read/write";
    }
    return s;
}


bool MemoryWatch::is_global() const
{
    assert(watchPoint_);
    return watchPoint_->is_global();
}


ValueWatch::ValueWatch
(
    RefPtr<DebugSymbol>  sym,
    RelType              rel,
    RefPtr<SharedString> val
)
  : MemoryWatch()
  , sym_(sym)
  , rel_(rel)
  , val_(val)
{
}


ValueWatch::~ValueWatch() throw()
{
}


const char* ValueWatch::name() const
{
    return "value_change";
}


word_t ValueWatch::cookie() const
{
    return 0;
}


void ValueWatch::execute_impl
(
    Debugger&       engine,
    Thread*         thread,
    BreakPoint*     breakpoint
)
{
    if (!in_scope(CHKPTR(thread)))
    {
        return;
    }

    sym_->read(0, 0);

    if (!sym_->value())
    {
        return;
    }
    DataType* type = CHKPTR(sym_->type());
    const int res = type->compare(sym_->value()->c_str(), val_->c_str());

    bool shouldBreak = false;
    switch (rel_)
    {
    case EQ: shouldBreak = (res == 0);
        break;

    case NEQ: shouldBreak = (res != 0);
        break;

    case LT: shouldBreak = (res < 0);
        break;

    case LTE: shouldBreak = (res <= 0);
        break;

    case GT: shouldBreak = (res > 0);
        break;

    case GTE: shouldBreak = (res >= 0);
        break;
    }

    if (shouldBreak)
    {
        RefPtr<SharedString> str = thread_get_event_description(*thread);

        if (str)
        {
            str = str->append(" met ");
            str = str->append(description_impl().c_str());

            thread_set_event_description(*thread, str);
        }

        MemoryWatch::execute_impl(engine, thread, breakpoint);
    }
}


std::string ValueWatch::description_impl() const
{
    std::string s("condition: ");

    assert(sym_.get());
    s += sym_->name()->c_str();

    static const char* relop[] = { "==", "!=", "<", "<=", ">", ">=" };
    s += ' ';
    s += relop[rel_];

    assert(val_.get());
    s += ' ';
    s += val_->c_str();

    return s;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
