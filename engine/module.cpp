//
// $Id: module.cpp 714 2010-10-17 10:03:52Z root $
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
#include <sys/stat.h>
#include <iostream>
#include "dharma/symbol_util.h"
#include "dharma/system_error.h"
#include "zdk/breakpoint_util.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/switchable.h"
#include "zdk/thread_util.h"
#include "zdk/zobject_scope.h"
#include "module.h"


using namespace std;


ModuleImpl::ModuleImpl(SymbolTable& symTable)
    : Persistent("Module")
    , symTable_(&symTable)
    , name_(symTable.filename())
    , addr_(symTable.addr())
    , upper_(symTable.upper())
    , lastModified_(0)
{
    assert(name_);

    struct stat stbuf;
    memset(&stbuf, 0, sizeof stbuf);

    while ((stat(name_->c_str(), &stbuf) < 0) && (errno == EINTR))
    {
        // empty; just loop
    }
    lastModified_ = stbuf.st_mtime;
}


/**
 * ctor for de-persisting from a stream
 */
ModuleImpl::ModuleImpl(const char* name)
    : Persistent("Module")
    , addr_(0)
    , upper_(0)
    , lastModified_(0)
{
    assert(name);
    name_ = shared_string(name);
}


ModuleImpl::~ModuleImpl() throw()
{
}


SharedString* ModuleImpl::name() const
{
    assert(name_);
    return name_.get();
}


void ModuleImpl::set_name(const char* name)
{
    assert(name);
    assert(!name_);

    name_ = shared_string(name);
}


void ModuleImpl::add_breakpoint_image(RefPtr<BreakPointImage> img)
{
    assert(img.get());
    breakpoints_.insert(make_pair(img->addr(), img));
}


off_t ModuleImpl::adjustment() const
{
    if (RefPtr<SymbolTable> table = symTable_.ref_ptr())
    {
        return table->adjustment();
    }
    return 0;
}


addr_t ModuleImpl::addr() const
{
    return addr_;
}


addr_t ModuleImpl::upper() const
{
    return upper_;
}


time_t ModuleImpl::last_modified() const
{
    return lastModified_;
}


SymbolTable* ModuleImpl::symbol_table_list() const
{
    RefPtr<SymbolTable> table = symTable_.ref_ptr();
    return CHKPTR(table.get());
}


/**
 * Callback received when a word_t is de-persisted from a file
 */
void ModuleImpl::on_word(const char* name, word_t value)
{
    if (strcmp(name, "addr") == 0)
    {
        assert(addr_ == 0);
        addr_ = value;
    }
    else if (strcmp(name, "upper") == 0)
    {
        assert(upper_ == 0);
        upper_ = value;
    }
    else if (strcmp(name, "modt") == 0)
    {
        assert(lastModified_ == 0);
        lastModified_ = value;
    }
    else
    {
        Persistent::on_word(name, value);
    }
}


size_t ModuleImpl::on_object_begin
(
    InputStream* stream,
    const char*  name,
    uuidref_t    uuid,
    size_t       size
)
{
    if (!uuid_equal(BreakPointImage::_uuid(), uuid))
    {
        return Persistent::on_object_begin(stream, name, uuid, size);
    }
    else
    {
        RefPtr<BreakPointImage> image = new BreakPointImage();
        size_t bytesRead = 0;
        for (; bytesRead != size; )
        {
            bytesRead += stream->read(image.get());
        }
        add_breakpoint_image(image);
        return bytesRead;
    }
}


/**
 * Persist to stream
 */
size_t ModuleImpl::write(OutputStream* stream) const
{
    size_t nbytes = stream->write_word("addr", addr_);
    nbytes += stream->write_word("upper", upper_);
    nbytes += stream->write_word("modt", lastModified_);

    // save breakpoints
    BreakPointMap::const_iterator i = breakpoints_.begin();
    for (; i != breakpoints_.end(); ++i)
    {
        nbytes += stream->write_object("", (*i).second.get());
    }

    return nbytes;
}


namespace
{
    struct AddrHelper : public EnumCallback<addr_t>
    {
        addr_t& addr_;

        explicit AddrHelper(addr_t& addr) : addr_(addr) {}

        void notify(addr_t addr) { addr_ = addr; }
    };
}


/**
 * Restores the saved breakpoints for the MODULE, using the map
 * of breakpoint images stored in THIS module
 */
void ModuleImpl::restore(Debugger& debugger, Process& proc, Module& module)
{
    assert(name_.get());
    assert(module.name()->is_equal(name_->c_str()));

    if (breakpoints_.empty())
    {
        return;
    }
    // NOTE: it is ok to use default thread, since we're
    // only restoring the GLOBAL breakpoints
    Thread* const thread = proc.get_thread(DEFAULT_THREAD);
    if (!thread || !CHKPTR(thread)->is_live())
    {
        return;
    }
    SymbolTable* table = CHKPTR(module.symbol_table_list());

    if (!table->is_loaded() && table->is_dynamic())
    {
        // NOTE: is_dynamic() may load the table as a the side-effect
        return;
    }
#ifdef DEBUG
    clog << "Restoring breakpoints in: " << name_->c_str() << endl;
#endif
    // The module may now be loaded at a different memory
    // address than it was the last time we ran the debugged
    // program, need to adjust the offset.
    const word_t offs = module.addr() - this->addr();

    // iterate thru breakpoint images
    BreakPointMap::iterator i = breakpoints_.begin();
    for (; i != breakpoints_.end(); ++i)
    {
        assert(i->first >= this->addr());
        restore_breakpoint(debugger,
                           i->second,   // breakpoint image
                           i->first,    // address
                           thread, module, table, offs);
    }
}


void ModuleImpl::restore_breakpoint(
    Debugger& debugger,
    RefPtr<BreakPointImage> img,
    addr_t bpntAddr,
    Thread* thread,
    Module& module,
    SymbolTable* table,
    word_t offs)
{
    assert(table);
    // todo: come up with some way to match saved thread
    // task IDs with the threads that are currently running.
    if (img->type() != BreakPoint::GLOBAL)
    {
        // clog << "per-thread breakpoint skipped\n";
        return;
    }
    addr_t addr = 0;

    //
    // has the module changed since last debugged?
    //
    if (module.last_modified() <= this->last_modified())
    {
        addr = bpntAddr + offs; //nope, OK to use addr
    }
    else
    {
        // module has changed, insert breakpoint by src line
    #ifdef DEBUG
        clog << module.name() << " has changed\n";
    #endif
        AddrHelper helper(addr);

        for (; table && !addr; table = table->next())
        {
            if (table->is_dynamic())
            {
                continue;
            }
            size_t lnum = img->line();
            table->enum_addresses_by_line(img->file(), lnum, &helper);
        }
    }
    if (!addr)
    {
        return;
    }
    try
    {
        const bool defer = /* img->is_deferred() || */ (module.addr() == 0);

        if (defer)
        {
            RefPtr<Symbol> sym;
            // todo: implement Module::lookup_symbol to do exactly this?
            for (table = module.symbol_table_list();
                 table && !sym;
                 table = table->next())
            {
                sym = table->lookup_symbol(addr);
            }
            if (sym)
            {
                sym->set_deferred_breakpoint(BreakPoint::GLOBAL);
            }
        }
        else if (debugger.set_user_breakpoint(get_runnable(thread), addr))
        {
    #ifdef DEBUG
            clog << "restored: 0x" << hex << addr << dec;
    #endif
        }

    #ifdef DEBUG
        RefPtr<SharedString> name = img->symbol_name();
        if (name.get())
        {
            clog << " (" << name->c_str() << ")";
        }
        clog << endl;
    #endif

        if (!defer)
        {
            if (!img->is_enabled())
            {
                disable_user_breakpoint_actions(debugger, addr);
            }
            // is the breakpoint conditional?
            if (!img->condition().empty() || img->activation_counter())
            {
                // set the conditions
                set_breakpoint_conditions(
                    debugger,
                    addr,
                    img->condition(),
                    img->activation_counter(),
                    img->auto_reset());
            }
        }
    }
    catch (const exception& e)
    {
        cerr << "Could not restore breakpoint at 0x";
        cerr << hex << addr << dec;
        cerr << ": " << e.what() << endl;
    }
}



namespace
{
    /**
     * Helper for enumerating compilation units for a given module.
     */
    class ZDK_LOCAL UnitEnumHelper : public EnumCallback<DebuggerPlugin*>
    {
        typedef EnumCallback<TranslationUnit*, bool> UnitEnumerator;

        RefPtr<Process>         process_;
        UnitEnumerator*         enumerator_;
        RefPtr<SharedString>    moduleName_;
        size_t                  unitCount_;

    public:
        UnitEnumHelper(UnitEnumerator* enumerator, RefPtr<SharedString> module)
            : enumerator_(enumerator)
            , moduleName_(module)
            , unitCount_(0)
        { }

        void notify(DebuggerPlugin* plugin)
        {
            if (DebugInfoReader* reader = interface_cast<DebugInfoReader*>(plugin))
            {
                unitCount_ += reader->lookup_unit_by_name(process_.get(),
                                                          moduleName_.get(),
                                                          NULL, // match all names
                                                          enumerator_);
            }
        }

        size_t unit_count() const { return unitCount_; }

    }; // UnitEnumHelper
}


Debugger* ModuleImpl::debugger() const
{
    Debugger* debugger = NULL;

    // get the debugger instance attached to the process that owns
    // this module's symbol table
    if (RefPtr<SymbolTable> table = symTable_.ref_ptr())
    {
        ZObjectScope scope;
        if (Process* process = table->process(&scope))
        {
            debugger = process->debugger();
        }
    }
    return debugger;
}


size_t ModuleImpl::enum_units(EnumCallback<TranslationUnit*, bool>* callback)
{
    if (Debugger* debugger = this->debugger())
    {
        UnitEnumHelper unitEnumHelper(callback, name_);
        return debugger->enum_plugins(&unitEnumHelper);
    }
    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
