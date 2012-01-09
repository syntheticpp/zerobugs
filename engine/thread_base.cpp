//
// $Id: thread_base.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include "zdk/check_ptr.h"
#include "zdk/zobject_impl.h"
#include "dharma/syscall_wrap.h"
#include "debugger_base.h"
#include "thread_base.h"
#include "target/cpu.h"
#ifdef HAVE_SYS_PARAM_H
 #include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
 #include <sys/user.h>
#endif

using namespace std;


////////////////////////////////////////////////////////////////
ThreadBase::~ThreadBase() throw()
{
}


////////////////////////////////////////////////////////////////
Debugger* ThreadBase::debugger() const
{
    if (RefPtr<Target> t = target())
    {
        return &t->debugger();
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
SymbolMap* ThreadBase::symbols() const
{
    if (RefPtr<Target> t = target())
    {
        return t->symbols();
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
void ThreadBase::set_user_data(const char* key, word_t val, bool replace)
{
    assert(key);
    UserDataMap::iterator i = userData_.find(key);
    if (i == userData_.end())
    {
        userData_.insert(make_pair(key, val));
    }
    else if (replace)
    {
        i->second = val;
    }
}


////////////////////////////////////////////////////////////////
bool ThreadBase::get_user_data(const char* key, word_t* val) const
{
    assert(key);
    UserDataMap::const_iterator i = userData_.find(key);

    if (i != userData_.end())
    {
        if (val)
        {
            *val = i->second;
        }
        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////
void ThreadBase::set_user_object (
    const char* key,
    ZObject*    object,
    bool        replace)
{
    assert(key);
    RefPtr<ZObject> objPtr(object);

    UserObjectMap::iterator i = userObj_.find(key);
    if (i == userObj_.end())
    {
        userObj_.insert(make_pair(key, objPtr));
    }
    else if (replace || !i->second)
    {
        i->second = objPtr;
    }
}



////////////////////////////////////////////////////////////////
ZObject* ThreadBase::get_user_object(const char* key) const
{
    assert(key);
    UserObjectMap::const_iterator i = userObj_.find(key);
    return (i == userObj_.end()) ? NULL : i->second.get();
}


////////////////////////////////////////////////////////////////
size_t ThreadBase::enum_user_regs(EnumCallback<Register*>* cb)
{
    if (RefPtr<Target> t = target())
    {
        return t->enum_user_regs(*this, cb);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
size_t ThreadBase::enum_fpxregs(EnumCallback<Register*>* cb)
{
    if (RefPtr<Target> t = target())
    {
        return t->enum_fpu_regs(*this, cb);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
ZObject* ThreadBase::regs(ZObject* gregs) const
{
    if (gregs)
    {
        interface_cast<Regs<>&>(*gregs);
        regs_ = gregs;
    }
    return regs_.get();
}


////////////////////////////////////////////////////////////////
ZObject* ThreadBase::fpu_regs(ZObject* fpuRegs) const
{
    if (fpuRegs)
    {
        interface_cast<FpuRegs<>&>(*fpuRegs);
        fpuRegs_ = fpuRegs;
    }
    return fpuRegs_.get();
}


////////////////////////////////////////////////////////////////
long ThreadBase::syscall_num() const
{
    if (RefPtr<Target> t = target())
    {
        return t->syscall_num(*this);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
word_t ThreadBase::result() const
{
    if (RefPtr<Target> t = target())
    {
        return t->result(*this);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
int64_t ThreadBase::result64() const
{
    if (RefPtr<Target> t = target())
    {
        return t->result64(*this);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
long double ThreadBase::result_double(size_t size) const
{
    if (RefPtr<Target> t = target())
    {
        return t->result_double(*this, size);
    }
    return .0;
}


////////////////////////////////////////////////////////////////
reg_t ThreadBase::read_register(int n, bool readFrame) const
{
    reg_t r = 0;

    if (RefPtr<Target> t = target())
    {
        if (!readFrame || !t->read_register(*this, n, readFrame, r))
        {
            t->read_register(*this, n, false, r);
        }
    }
    return r;
}


////////////////////////////////////////////////////////////////
addr_t ThreadBase::program_count() const
{
    if (RefPtr<Target> tgt = target())
    {
        return tgt->program_count(*this);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
addr_t ThreadBase::stack_pointer() const
{
    if (RefPtr<Target> tgt = target())
    {
        return tgt->stack_pointer(*this);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
addr_t ThreadBase::frame_pointer() const
{
    if (RefPtr<Target> tgt = target())
    {
        return tgt->frame_pointer(*this);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
void ThreadBase::read_data (
    addr_t  addr,
    word_t* buf,
    size_t  count,
    size_t* wordsRead
    ) const
{
    if (RefPtr<Target> t = target())
    {
        t->read_memory(this->lwpid(),
                       Target::DATA_SEGMENT,
                       addr,
                       buf,
                       count,
                       wordsRead);
    }
    else if (wordsRead)
    {
        *wordsRead = 0;
    }
}


////////////////////////////////////////////////////////////////
void ThreadBase::write_data(addr_t addr, const word_t* buf, size_t count)
{
    if (RefPtr<Target> t = target())
    {
        t->write_memory(lwpid(), Target::DATA_SEGMENT, addr, buf, count);
    }
}


////////////////////////////////////////////////////////////////
void ThreadBase::read_code (
    addr_t  addr,
    word_t* buf,
    size_t  count,
    size_t* wordsRead
    ) const
{
    if (RefPtr<Target> t = target())
    {
        t->read_memory(lwpid(),
                       Target::CODE_SEGMENT,
                       addr,
                       buf,
                       count,
                       wordsRead);
    }
    else if (wordsRead)
    {
        *wordsRead = 0;
    }
}


////////////////////////////////////////////////////////////////
void ThreadBase::write_code(addr_t addr, const word_t* buf, size_t nwords)
{
    if (RefPtr<Target> t = target())
    {
        t->write_memory(lwpid(), Target::CODE_SEGMENT, addr, buf, nwords);
    }
}


////////////////////////////////////////////////////////////////
Process* ThreadBase::process() const
{
    if (RefPtr<Target> t = target())
    {
        return t->process();
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
bool ThreadBase::is_32_bit() const
{
    if (RefPtr<Target> t = target())
    {
        return t->kind() == Target::K_NATIVE_32BIT;
    }
    return false;
}


////////////////////////////////////////////////////////////////
bool ThreadBase::is_syscall_pending(addr_t pc) const
{
    return thread_syscall_pending(*this, pc);
}


////////////////////////////////////////////////////////////////
DebugRegs* ThreadBase::debug_regs()
{
    throw logic_error("debug_regs(): invalid thread class");
}


////////////////////////////////////////////////////////////////
const char* ThreadBase::name() const
{
    if (RefPtr<Target> t = target())
    {
        name_ = t->thread_name(lwpid());
        return name_.c_str();
    }
    return filename();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
