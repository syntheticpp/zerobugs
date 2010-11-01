//
// $Id: process.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/cstdint.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/tokenizer.hpp>
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "dharma/canonical_path.h"
#include "dharma/process_name.h"
#include "debugger_base.h"
#include "history.h"
#include "process.h"
#include "thread.h"
#include "target/target.h"


using namespace std;



////////////////////////////////////////////////////////////////
ProcessImpl::~ProcessImpl() throw()
{
}


////////////////////////////////////////////////////////////////
ProcessImpl::ProcessImpl
(
    Target& target,
    pid_t pid,
    ProcessOrigin origin,
    const string* cmd,
    const char* name
)
    : target_(&target)
    , pid_(pid)
    , origin_(origin)
    , watches_(new WatchListImpl)
{
    if (name)
    {
        name_ = shared_string(name);
    }
    if (cmd)
    {
        typedef boost::escaped_list_separator<char> Delim;

        typedef boost::tokenizer<Delim> Tokenizer;

        Tokenizer tok(*cmd, Delim('\\', ' ', '\"'));

        string tmp;

        Tokenizer::const_iterator it = tok.begin();
        for (; it != tok.end(); ++it)
        {
            if (tmp.empty())
            {
                tmp += "\"";
                tmp += canonical_path(it->c_str());
                tmp += "\"";
            }
            else
            {
                tmp += " ";
                tmp += *it;
            }
        }

        cmdline_ = shared_string(tmp);
    }
}


////////////////////////////////////////////////////////////////
Thread* ProcessImpl::get_thread(lwpid_t pid, unsigned long tid)
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        Thread* thread = target->get_thread(pid, tid);

        assert (!thread || thread->process() == this);
        return thread;
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
BreakPointManager* ProcessImpl::breakpoint_manager() const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        return target->debugger().breakpoint_manager(pid_);
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
const char* ProcessImpl::name() const
{
    if (!name_)
    {
        name_ = shared_string(realpath_process_name(pid()));
    }
    return name_->c_str();
}


////////////////////////////////////////////////////////////////
void ProcessImpl::set_name(const char* name)
{
    if (!name)
    {
        name_.reset();
    }
    else
    {
        string path(name);
      /*
        if (RefPtr<Target> t = target())
        {
            t->map_path(path);
        }
      */
        name_ = shared_string(path);
    }
}

////////////////////////////////////////////////////////////////
SymbolMap* ProcessImpl::symbols() const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        return target->symbols();
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
namespace
{
    /**
     * Helper callback functor user by DebuggerBase::enum_modules,
     * forwards notifications to EnumCallback<Module*> instance
     */
    class EnumModuleHelper : public EnumCallback<SymbolTable*>
    {
    public:
        EnumModuleHelper(EnumCallback<Module*>* cb)
            : cb_(cb), count_(0) {}

        void notify(SymbolTable* symTable)
        {
            assert(symTable); // pre-condition

            if (cb_)
            {
                if (RefPtr<Module> module = symTable->module())
                {
                    cb_->notify(module.get());
                }
            }

            ++count_;
        }

        size_t count() const { return count_; }

    private:
        EnumCallback<Module*>* cb_;
        size_t count_;
    };
}


////////////////////////////////////////////////////////////////
size_t ProcessImpl::enum_modules(EnumCallback<Module*>* callback)
{
    EnumModuleHelper helperCB(callback);

    if (RefPtr<Target> target = target_.ref_ptr())
    {
        if (SymbolMap* symbols = target->symbols())
        {
            symbols->enum_symbol_tables(&helperCB);
            symbols->enum_needed_tables(&helperCB);
        }
    }
    return helperCB.count();
}


////////////////////////////////////////////////////////////////
size_t ProcessImpl::enum_threads(EnumCallback<Thread*>* cb)
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        return target->enum_threads(cb);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
const char* const* ProcessImpl::environment() const
{
    if (environ_.empty())
    {
        if (RefPtr<Target> target = target_.ref_ptr())
        {
            target->read_environment(environ_);
        }
    }
    return environ_.cstrings();
}


////////////////////////////////////////////////////////////////
void ProcessImpl::set_environment(const char* const* env)
{
    SArray(env).swap(environ_);
}


////////////////////////////////////////////////////////////////
SymbolTable* ProcessImpl::vdso_symbol_tables() const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        return target->vdso_symbol_tables().get();
    }
    return 0;
}


////////////////////////////////////////////////////////////////
void ProcessImpl::read_cmdline() const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        const string& cmd = target->command_line();

        if (!cmd.empty())
        {
            cmdline_ = shared_string(cmd);
        }
    }
}


////////////////////////////////////////////////////////////////
SharedString* ProcessImpl::command_line() const
{
    if (!cmdline_)
    {
        read_cmdline();
    }
    return cmdline_.get();
}


////////////////////////////////////////////////////////////////
void ProcessImpl::read_data (
    addr_t  addr,
    word_t* buf,
    size_t  bufLen,
    size_t* readLen
    ) const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        target->read_data(addr, buf, bufLen, readLen);
    }
    else if (readLen)
    {
        *readLen = 0;
    }
}


////////////////////////////////////////////////////////////////
void ProcessImpl::write_data(addr_t addr, const long* buf, size_t len)
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        target->write_data(addr, buf, len);
    }
    else
    {
        cerr << __func__ << ": no debug target associated with this process\n";
    }
}


////////////////////////////////////////////////////////////////
void ProcessImpl::read_code (
    addr_t  addr,
    word_t* buf,
    size_t  bufLen,
    size_t* readLen
    ) const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        target->read_code(addr, buf, bufLen, readLen);
    }
    else if (readLen)
    {
        *readLen = 0;
    }
}


////////////////////////////////////////////////////////////////
void ProcessImpl::write_code(addr_t addr, const long* buf, size_t len)
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        target->write_code(addr, buf, len);
    }
    else
    {
        cerr << __func__ << ": no debug target associated with this process\n";
    }
}


////////////////////////////////////////////////////////////////
bool ProcessImpl::is_attached(Thread* thread) const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        return target->is_attached(thread);
    }
    return false;
}


////////////////////////////////////////////////////////////////
TypeSystem* ProcessImpl::type_system() const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        return target->type_system();
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
Debugger* ProcessImpl::debugger() const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        return &target->debugger();
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
Target* ProcessImpl::target() const
{
    return target_.ref_ptr().get();
}


////////////////////////////////////////////////////////////////
void ProcessImpl::set_watches(const RefPtr<WatchList>& watches)
{
    watches_ = watches;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
