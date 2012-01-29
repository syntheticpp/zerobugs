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

#include "zdk/config.h"
#include "zdk/check_ptr.h"
#include "zdk/export.h"
#include "zdk/shared_string_impl.h"
#include "zdk/zero.h"
#include "zdk/zobject_scope.h"
#include "zdk/log.h"

#ifdef HAVE_UNISTD_H
 #include <unistd.h>
#endif
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "generic/lock.h"
#include "dharma/path.h"
#include "dharma/environ.h"
#include "dharma/syscall_wrap.h"
#include "history.h"
#include "target/target.h"


using namespace std;


// Helper enumeration callbacks used by the HistoryEntry
namespace
{
    /**
     * This functor builds a collection of modules for a given
     * HistoryEntry. Each entry corresponds to a debugged program.
     */
    class ZDK_LOCAL ModuleListBuilder : public EnumCallback<Module*>
    {
        // non-copyable, non-assignable
        ModuleListBuilder(const ModuleListBuilder&);
        ModuleListBuilder& operator=(const ModuleListBuilder&);

    public:
        explicit ModuleListBuilder(HistoryEntryImpl& entry)
            : entry_(entry)
        { }

        void notify(Module* mod)
        {
            if (ModuleImpl* impl = interface_cast<ModuleImpl*>(mod))
            {
                impl->erase_breakpoint_images();
                entry_.add_module(impl);
            }
        }

    private:
        HistoryEntryImpl& entry_;
    };


    /**
     * This functor builds a collection of breakpoint images for
     * each module. A BreakPointImage is a persistent object that
     * we can use to restore a user breakpoint.
     */
    class ZDK_LOCAL BreakPointListBuilder
        : public EnumCallback<volatile BreakPoint*>
    {
    public:
        explicit BreakPointListBuilder(HistoryEntryImpl& entry)
            : entry_(entry)
        {}

        void notify(volatile BreakPoint* bpnt)
        {
            assert(bpnt);

            // interested in user-defined breakpoints only
            if (bpnt->enum_actions("USER") == 0)
            {
                return;
            }

            RefPtr<ModuleImpl> module;

            try
            {
                // this may fail if breakpoint is in a debuggee thread
                // that went AWOL
                if (Symbol* symbol = bpnt->symbol())
                {
                    ZObjectScope scope;
                    if (SymbolTable* table = symbol->table(&scope))
                    {
                        RefPtr<SharedString> modName = table->filename();
                        module = entry_.get_module(modName);
                    }
                }
            }
            catch (const exception& e)
            {
                clog << "Non-critical error in saving breakpoint: " << e.what() << endl;
            }
            // we might not know what symbol is associated
            // with the breakpoint, and thus the above block
            // may fail; look module up by address
            if (!module)
            {
                module = entry_.get_module(bpnt->addr());
            }
            if (module)
            {
                RefPtr<BreakPointImage> img(new BreakPointImage(*bpnt));
                module->add_breakpoint_image(img);
            }
        }

    private:
        HistoryEntryImpl& entry_;
    };
} // namespace


/**
 * Make sure directory exists. If not, create it with specified mode.
 */
static void ensure_dir(const std::string& path, int mode)
{
    struct stat st = { 0 };
    sys::stat(sys::dirname(path.c_str()), st);

    sys::ImpersonationScope impersonate(st.st_uid);
    sys::mkdir(path, mode);
}


/**
 * This ctor is used when reading from an input stream.
 */
HistoryEntryImpl::HistoryEntryImpl(

    WeakPtr<ObjectFactory>  factory,
    const char*             name
    )
    : Persistent("HistoryEntry")
    , pid_(0)
    , lastDebugged_(0)
    , isLive_(false)
    , origin_(ORIGIN_SYSTEM)
    , prop_(new Settings(factory))
    , watches_(new WatchListImpl)
    , factory_(factory)
{
    assert(name);
    name_.assign(name);
}


/**
 * This ctor snapshots the state of the debugged program.
 */
HistoryEntryImpl::HistoryEntryImpl(Process& process)
    : Persistent("HistoryEntry")
    , pid_(process.pid())
    , lastDebugged_(time(NULL))
    , isLive_(process.origin() != ORIGIN_CORE)
    , name_(process.name())
    , env_(process.environment())
    , origin_(process.origin())
    , prop_(new Settings(NULL))
    , watches_(interface_cast<WatchListImpl*>(&process))
    , factory_(NULL)
{
    if (RefPtr<Target> target = process.target())
    {
        targetParam_ = target->param();
    }
    if (process.command_line())
    {
        cmdLine_ = process.command_line()->c_str();
    }

    ModuleListBuilder moduleListBuilder(*this);
    process.enum_modules(&moduleListBuilder);

    if (isLive_)
    {
        BreakPointListBuilder bpntListBuilder(*this);
        if (BreakPointManager* bpntMgr = process.breakpoint_manager())
        {
            bpntMgr->enum_breakpoints(&bpntListBuilder);
        }
    }
}


HistoryEntryImpl::~HistoryEntryImpl() throw()
{
}


const char* HistoryEntryImpl::command_line() const
{
    return cmdLine_.empty() ? NULL : cmdLine_.c_str();
}


void HistoryEntryImpl::set_command_line(const char* cmd)
{
    if (cmd)
    {
        cmdLine_ = cmd;
    }
    else
    {
        cmdLine_.clear();
    }
}


void HistoryEntryImpl::set_environ(const char* const* env)
{
    SArray(env).swap(env_);
}


/**
 * Persist to stream
 */
size_t HistoryEntryImpl::write(OutputStream* output) const
{
    size_t nbytes = output->write_word("pid", pid_)
                  + output->write_word("last", lastDebugged_)
                  + output->write_word("live", isLive_)
                  + output->write_word("orig", origin_);

    if (!cmdLine_.empty())
    {
        nbytes += output->write_string("cmdl", cmdLine_.c_str());
    }
    if (!path_.empty())
    {
        nbytes += output->write_string("path", path_.c_str());
    }
    if (!targetParam_.empty())
    {
        nbytes += output->write_string("target", targetParam_.c_str());
    }
    //
    // save watch list
    //
    if (watches_)
    {
        WatchListImpl::const_iterator i = watches_->begin(), end = watches_->end();
        int count = 0;
        for (; i != end; ++i, ++count)
        {
            nbytes += output->write_string("watch", i->c_str());
        }
    }
    //
    // save settings for all modules in this entry
    //
    Modules::const_iterator i = modules_.begin();
    for (; i != modules_.end(); ++i)
    {
        const ModuleImpl* module = (*i).second.get();
        if (!module || !module->last_modified())
        {
            continue;
        }
        size_t tmp = output->write_object(module->name()->c_str(), module);
        nbytes += tmp;
    }
    //
    // write additional properties
    //
    if (prop_)
    {
        nbytes += output->write_object("prop", prop_.get());
    }

    //
    // write environment
    //
    if (!path_.empty())
    {
        try
        {
            ensure_dir(path_, 0750);
            env::write(path_ + "/environ", environ());
        }
        catch (const exception& e)
        {
            cerr << path_ + "/environ: " << e.what() << endl;
        }
    }
    return nbytes;
}


void HistoryEntryImpl::add_module(const RefPtr<ModuleImpl>& mod)
{
    modules_.insert(std::make_pair(CHKPTR(mod)->name(), mod));
}


RefPtr<ModuleImpl>
HistoryEntryImpl::get_module(const RefPtr<SharedString>& name) const
{
    RefPtr<ModuleImpl> module;

    Modules::const_iterator i = modules_.find(name);
    if (i != modules_.end())
    {
        module = i->second;
    }
    return module;
}


RefPtr<ModuleImpl> HistoryEntryImpl::get_module(addr_t addr) const
{
    RefPtr<ModuleImpl> module;

    Modules::const_iterator i = modules_.begin();
    for (; i != modules_.end(); ++i)
    {
        if ((i->second->addr() >= addr) && (i->second->upper() < addr))
        {
            module = i->second;
            break;
        }
    }
    return module;
}


void HistoryEntryImpl::on_word(const char* name, word_t value)
{
    assert(name);

    if (strcmp(name, "pid") == 0)
    {
        assert(pid_ == 0);
        pid_ = value;
    }
    else if (strcmp(name, "last") == 0)
    {
        assert(lastDebugged_ == 0);
        lastDebugged_ = value;
    }
    else if (strcmp(name, "live") == 0)
    {
        isLive_ = (value != 0);
    }
    else if (strcmp(name, "orig") == 0)
    {
        origin_ = static_cast<ProcessOrigin>(value);
    }
    else
    {
        Persistent::on_word(name, value);
    }
}


void HistoryEntryImpl::on_string(const char* name, const char* str)
{
    assert(name);

    if (strcmp(name, "cmdl") == 0)
    {
        if (str)
        {
            cmdLine_.assign(str);
        }
    }
    else if (strcmp(name, "path") == 0)
    {
        if (str)
        {
            path_.assign(str);
            ensure_dir(path_, 0750);
        }
    }
    else if (strcmp(name, "target") == 0)
    {
        if (str)
        {
            targetParam_.assign(str);
        }
    }
    else if (strcmp(name, "watch") == 0)
    {
        CHKPTR(watches_)->add(str);
    }
    else
    {
        Persistent::on_string(name, str);
    }
}


size_t HistoryEntryImpl::on_object_begin
(
    InputStream*    input,
    const char*     name,
    uuidref_t       uuid,
    size_t          size
)
{
    if (uuid_equal(Settings::_uuid(), uuid))
    {
        prop_ = new Settings(factory_);

        size_t nbytes = 0;
        for (; nbytes < size; )
        {
            size_t n = input->read(prop_.get());
            if (!n)
            {
                // this can happen if say, some object is not registered
                break;
            }

            nbytes += n;
        }
    }
    else if (uuid_equal(ModuleImpl::_uuid(), uuid))
    {
        RefPtr<ModuleImpl> module = new ModuleImpl(name);

        size_t nbytes = 0;
        for (; nbytes < size; )
        {
            size_t n = input->read(module.get());
            assert(n);

            nbytes += n;
        }
        assert(nbytes == size);

        add_module(module);
    }
    else
    {
        Persistent::on_object_begin(input, name, uuid, size);
    }
    return size;
}


void HistoryEntryImpl::set_path(const std::string& path)
{
    assert(path_.empty()); // set once and only once
    assert(pid_);

    ostringstream dir;
    dir << path;
    if (!path.empty() && path[path.size() - 1] != '/')
    {
        dir << '/';
    }
    dir << pid_;
    path_ = dir.str();

    ensure_dir(path_, 0750);
}


void HistoryEntryImpl::read_environ()
{
    if (!path_.empty())
    {
        SArray env;
        env::read(path_ + "/environ", env);

        env_.swap(env);
    }
}


void HistoryEntryImpl::destroy()
{
    if (!path_.empty())
    {
        try
        {
            sys::rmdir(path_, true);
        }
        catch (const exception& e)
        {
            cerr << __func__ << ": " << e.what() << endl;
        }
    }
}


////////////////////////////////////////////////////////////////
//
// HistoryImpl
//
HistoryImpl::HistoryImpl(WeakPtr<ObjectFactory> factory, const string& path)
    : Persistent("History")
    , path_(path)
    , factory_(factory)
{
}


HistoryImpl::~HistoryImpl() throw()
{
}


size_t HistoryImpl::write(OutputStream* output) const
{
    Lock<Mutex> lock(mutex_);
    size_t nbytes = 0;

    if (!path_.empty())
    {
        nbytes += output->write_string("path", path_.c_str());
    }
    Queue::const_iterator i = queue_.begin();
    for (; i != queue_.end(); ++i)
    {
        size_t tmp = output->write_object((*i)->name(), (*i).get());

        nbytes += tmp;
    }
    dbgout(0) << "history saved, " << nbytes << " bytes" << endl;

    return nbytes;
}


void HistoryImpl::add_entry(Process* process)
{
    assert(process);

    if (!process)
    {
        return;
    }

    if (Thread* thread = process->get_thread(DEFAULT_THREAD))
    {
        if (thread->is_forked() || thread->is_execed())
        {
            return;
        }
    }
    RefPtr<HistoryEntryImpl> entry = new HistoryEntryImpl(*process);

    if ((entry->name() == 0) || (entry->name()[0] == 0))
    {
        return;
    }
    assert(!path_.empty());
    entry->set_path(path_);

    Lock<Mutex> lock(mutex_);

    Queue::iterator i = queue_.begin();
    for (; i != queue_.end(); ++i)
    {
        if (strcmp((*i)->name(), entry->name()) == 0)
        {
            if ((*i)->pid() != entry->pid())
            {
                (*i)->destroy();
            }

            dbgout(0) << __func__ << ": erasing entry " << entry->name() << endl;

            queue_.erase(i);
            break;
        }
    }
    queue_.push_front(entry);
}


namespace
{
    class ZDK_LOCAL ProcessHistoryHelper : public EnumCallback<Process*>
    {
        typedef void (HistoryImpl::*CallbackPtr)(Process*);

        HistoryImpl* history_;
        CallbackPtr  callback_;

    public:
        ProcessHistoryHelper(HistoryImpl* h, CallbackPtr cb)
            : history_(h), callback_(cb)
        { }

        void notify(Process* process)
        {
            (history_->*callback_)(process);
        }
    };
} // namespace


void HistoryImpl::take_snapshot(Debugger& debugger)
{
    ProcessHistoryHelper historyHelper(this, &HistoryImpl::add_entry);
    debugger.enum_processes(&historyHelper);
}



RefPtr<HistoryEntryImpl>
HistoryImpl::get_entry_impl(const char* name) const
{
    assert(name);

    RefPtr<HistoryEntryImpl> entry;

    Lock<Mutex> lock(mutex_);

    Queue::const_iterator i = queue_.begin();
    for (; i != queue_.end(); ++i)
    {
        if (strcmp((*i)->name(), name) == 0)
        {
            entry = *i;
            break;
        }
    }
    return entry;
}



template<typename T>
inline static void
check_range(const char* fun, const T& list, size_t n)
{
    if (n >= list.size())
    {
        ostringstream err;
        err << fun << "[" << n << "]: out of range, size=";
        err << list.size();

        throw out_of_range(err.str());
    }
}

const HistoryEntry* HistoryImpl::entry(size_t n) const
{
    Lock<Mutex> lock(mutex_);
    check_range(__func__, queue_, n);
    return queue_[n].get();
}


void HistoryImpl::remove_entry(size_t n)
{
    Lock<Mutex> lock(mutex_);
    check_range(__func__, queue_, n);
    queue_[n]->destroy();

    queue_.erase(queue_.begin() + n);
}


/**
 * De-persist one history entry
 */
size_t HistoryImpl::on_object_begin
(
    InputStream*    input,
    const char*     name,
    uuidref_t       uuid,
    size_t          size
)
{
    if (!uuid_equal(HistoryEntry::_uuid(), uuid))
    {
        Persistent::on_object_begin(input, name, uuid, size);
    }
    else
    {
        RefPtr<HistoryEntryImpl> entry = new HistoryEntryImpl(factory_, name);

        size_t nbytes = 0;
        for (; nbytes < size; )
        {
            size_t tmp = input->read(entry.get());

            assert(tmp);
            nbytes += tmp;
        }
        assert(nbytes == size);

        entry->read_environ();

        Lock<Mutex> lock(mutex_);
        queue_.push_back(entry);
    }
    return size;
}


void HistoryImpl::on_string(const char* name, const char* str)
{
    assert(name);

    if (strcmp(name, "path") == 0)
    {
        if (str)
        {
            path_ = str;
            sys::mkdir(path_, 0750);
        }
    }
    else
    {
        Persistent::on_string(name, str);
    }
}



////////////////////////////////////////////////////////////////
//
// WatchListImpl
//
bool WatchListImpl::add(const char* watch)
{
    if (!watch)
    {
        return false;
    }
    return watches_.insert(watch).second;
}


void WatchListImpl::clear()
{
    watches_.clear();
}


size_t WatchListImpl::foreach(EnumCallback<const char*>* cb) const
{
    if (cb)
    {
        for (set<string>::const_iterator i = watches_.begin(); i != watches_.end(); ++i)
        {
            cb->notify(i->c_str());
        }
    }
    return watches_.size();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
