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
//
#include "zdk/config.h"
#include <algorithm>    // for std::sort
#include <stdexcept>
#include "zdk/assert.h"
#include "zdk/check_ptr.h"
#include "zdk/process.h"
#include "zdk/shared_string_impl.h"
#include "zdk/string.h"
#include "zdk/type_system.h"
#include "zdk/zero.h"
#include "zdk/zobject_scope.h"
#include "dharma/environ.h"
#include "dharma/path.h"
#include "dharma/symbol_util.h"
#include "elfz/public/binary.h"
#include "elfz/public/symbol_table.h"
#include "private/offset_symbol.h"
#include "private/predicates.h"
#include "private/symbol_addr.h"
#include "private/symbol_impl.h"
#include "private/symbol_table_impl.h"
#include "private/debug.h"
#include "unmangle/encoder.h"


using namespace std;
using namespace Predicate;

#define CHECK_LAZY_LOAD() lazy_load(__func__)
#define THREAD_SAFE() Lock<Mutex> lock(mutex_);



SymbolTableBase::SymbolTableBase
(
    SymbolTableEvents& e,
    Process* p,
    addr_t addr,
    addr_t upper
)
  : events_(&e)
  , process_(p)
  , addr_(addr)
  , upper_(upper)
  , adjust_(0)
{
}


Process* SymbolTableBase::process(ZObjectManager* mgr) const
{
    if (mgr)
    {
        if (RefPtr<Process> proc = process_.ref_ptr())
        {
            mgr->manage(proc.get());
            return proc.detach();
        }
    }
    return NULL;
}


static bool module_changed(const RefPtr<Module>& module, addr_t addr, addr_t upper)
{
    return module.is_null()
        || module->addr() != addr
        || module->upper()!= upper;
}


Module* SymbolTableBase::module()
{
    THREAD_SAFE();
    if (module_changed(module_, addr_, upper_))
    {
        module_ = events_->get_module(*this);

        // todo: module_->set_name(filename_)?
        // or assert(filename_.get() == module.name().get())?
    }
    return module_.get();
}


bool SymbolTableBase::set_deferred_breakpoint(
    Symbol&             sym,
    BreakPoint::Type    type,
    Runnable*           runnable,
    BreakPoint::Action* action)
{
    // method should only be used on tables that are not loaded
    assert(this->addr() == 0);
    assert(type != BreakPoint::ANY);

    const addr_t addr = sym.addr();
    IF_DEBUG(clog << __func__ << ": " << sym << endl);

    DeferredMap::iterator i = deferred_.find(addr);

    if (i != deferred_.end())
    {
        for (; i->first == addr; ++i)
        {
            if (i == deferred_.end())
            {
                break;
            }

            // already in the map?
            if (RefPtr<BreakPoint> bpnt = i->second.ref_ptr())
            {
                if (runnable == 0 || bpnt->thread() == runnable->thread())
                {
                    return bpnt->add_action(action);
                }
            }
        }
        i = deferred_.end();
    }

    if (BreakPoint* bpnt =
        events_->set_deferred_breakpoint(runnable, type, sym, action))
    {
        deferred_.insert(i, make_pair(addr, bpnt));
    }
    return true;
}


size_t SymbolTableBase::enum_deferred_breakpoints (
    EnumCallback2<BreakPoint*, const SymbolTable*>* callback
    ) const
{
    if (!deferred_.empty())
    {
        ensure_loaded();
    }

    DeferredMap::const_iterator i = deferred_.begin();
    for (; i != deferred_.end(); ++i)
    {
        if (callback)
        {
            if (RefPtr<BreakPoint> bpnt = i->second.lock())
            {
                callback->notify(bpnt.get(), this);
            }
        }
    }
    size_t count = deferred_.size();

    if (RefPtr<SymbolTable> nextTable = next())
    {
        count += nextTable->enum_deferred_breakpoints(callback);
    }
    return count;
}


SymbolTableImpl::~SymbolTableImpl() throw()
{
    // cancel async tasks that have not been picked up
    // for execution by the TaskPool yet
    sortByAddr_->done();
    sortByName_->done();
    sortByDemangledName_->done();

    wait_for_async_sort();
}


/**
 * Construct a lazy loading table
 */
SymbolTableImpl::SymbolTableImpl (
    Process*                proc,
    const string&           path,
    SymbolTableEvents&      events,
    addr_t                  addr,
    addr_t                  upper
    )
    : SymbolTableBase(events, proc, addr, upper)
    , filename_(shared_string(path))
    , dynamic_(false)
    , virtual_(false)
    , index_(-1)
    , sortByAddr_(new Delegate(this, &SymbolTableImpl::sort_by_addr))
    , sortByName_(new Delegate(this, &SymbolTableImpl::sort_by_name))
    , sortByDemangledName_(new Delegate(this, &SymbolTableImpl::sort_by_demangled_name))
{
    realname_ = filename_;
}


SymbolTableImpl::SymbolTableImpl (
    Process*                proc,
    RefPtr<SharedString>    filename,
    const ELF::SymbolTable& tbl,
    SymbolTableEvents&      events,
    addr_t                  vmemAddr,
    addr_t                  loadAddr,
    addr_t                  upper
    )
    : SymbolTableBase(events, proc, vmemAddr, upper)
    , name_(SharedStringImpl::create(tbl.header().name()))
    , filename_(filename)
    , dynamic_(tbl.header().type() == SHT_DYNSYM)
    , virtual_(false)
    , index_(0)
    , sortByAddr_(new Delegate(this, &SymbolTableImpl::sort_by_addr))
    , sortByName_(new Delegate(this, &SymbolTableImpl::sort_by_name))
    , sortByDemangledName_(new Delegate(this, &SymbolTableImpl::sort_by_demangled_name))
{
    assert(upper_);
    adjust_ = vmemAddr - loadAddr;
    realname_ = filename_;
}


void SymbolTableImpl::sort_by_name() const
{
    symbolsByName_.sort();
}


void SymbolTableImpl::sort_by_addr() const
{
    sort(symbols_.begin(), symbols_.end(), CompareAddr());
}


/**
 *  Attempt to start threads to sort by addr and name, respectively;
 *  if that fails, sort symbols synchronously.
 */
void SymbolTableImpl::sort_by_addr_and_name_async()
{
    // this method should be called only once, from read()
    assert(symbolsByName_.empty());

    symbolsByName_.assign(symbols_.begin(), symbols_.end());
    symbolsByDemangledName_.assign(symbols_.begin(), symbols_.end());

    TaskPool& tasks = SymbolTaskPool::instance();

    tasks.schedule(sortByAddr_);
    tasks.schedule(sortByName_);
    tasks.schedule(sortByDemangledName_);
}


SymAddrPtr SymbolTableImpl::create_symbol(
    const ELF::Symbol& elfSym,
    const char* name,
    TypeSystem* types)
{
    RefPtr<SharedString> sname =
            types ? types->get_string(name)
                  : SharedStringImpl::create(name);

    const addr_t addr = elfSym.value();
    return SymAddrPtr(new SymbolImpl(*this, elfSym, sname), addr);
}


/**
 * Read symbols from the specified ELF table.
 */
void SymbolTableImpl::read(const ELF::SymbolTable& tbl, bool notifyOnDone)
{
    events_->on_init(*this);
    symbols_.reserve(tbl.size());

    ZObjectScope scope;
    // the TypeSystem manager also acts as a string pool;
    // we pass it to create_symbol() below
    TypeSystem* types = interface_cast<TypeSystem*>(process(&scope));

    ELF::SymbolTable::const_iterator i = tbl.begin();
    const ELF::SymbolTable::const_iterator end = tbl.end();

    for (; i != end; ++i)
    {
        Symbol* prev = symbols_.empty() ? 0 : symbols_.back().get();
        const char* name = i->name();

        if (!name || !*name || i->value() == 0)
        {
            continue;
        }
        // hack: discard useless symbols
        if (*name == 'g' && strcmp(name, "gcc2_compiled.") == 0)
        {
            continue;
        }

        SymAddrPtr sym = create_symbol(*i, name, types);

        // prefer function symbols over non-functions, for same value
        if (prev && sym->value() == prev->value())
        {
            if (!prev->is_function() && sym->is_function())
            {
                symbols_.pop_back();
            }
            if (prev->is_function() && !sym->is_function())
            {
                continue;
            }
        }
        if (events_->on_symbol(*this, *sym))
        {
            symbols_.push_back(sym);
        }
    }
    sort_by_addr_and_name_async();

    if (notifyOnDone)
    {
        events_->on_done(*this);
    }
}



loff_t SymbolTableBase::adjustment() const
{
    // Need to fully load, we don't know the adjustment until
    // we load the table, since by definition it is the delta
    // between the address where loaded in mem and the load
    // address preferred by the shared object.
    ensure_loaded();

    return adjust_;
}



void SymbolTableBase::set_addr(addr_t addr, addr_t elfLoadAddr)
{
    assert((addr_ == 0) || (addr == addr_));

    adjust_ = addr - elfLoadAddr;
    const addr_t delta = addr - addr_;
    addr_ = addr;
    upper_ += delta;

    if (SymbolTableBase* nextTable = interface_cast<SymbolTableBase*>(next()))
    {
        nextTable->set_addr(addr, elfLoadAddr);
    }
}



SharedString* SymbolTableImpl::name() const
{
    CHECK_LAZY_LOAD();

    assert(name_);
    return name_.get();
}



SharedString* SymbolTableImpl::filename(bool follow) const
{
    if (follow)
    {
        assert(realname_);
        return realname_.get();
    }
    assert(filename_);
    return filename_.get();
}



void SymbolTableImpl::set_realname(const RefPtr<SharedString>& fname) const
{
    assert(fname);

    for (const SymbolTableImpl* table = this; table; table = table->next_impl())
    {
        table->realname_ = fname;
    }
}



bool SymbolTableImpl::is_dynamic() const
{
    if (is_lazy_stub())
    {
        CHECK_LAZY_LOAD();
        if (SymbolTable* table = next())
        {
            return table->is_dynamic();
        }
    }
    return dynamic_;
}



size_t SymbolTableImpl::size() const
{
    if (is_lazy_stub())
    {
        CHECK_LAZY_LOAD();

        if (SymbolTable* table = next())
        {
            return table->size();
        }
    }
    return symbols_.size();
}



void SymbolTableImpl::ensure_loaded() const
{
    lazy_load(__func__, false);
}


bool SymbolTableImpl::lazy_load(const char* func, bool warn) const
{
    THREAD_SAFE();

    if (!name_)
    {
        assert(is_lazy_stub());
        assert(filename());

        if (warn)
        {
            IF_DEBUG(clog << "Loading " << filename()->c_str() << endl);
        }

        // Set the name here to prevent recursion
        name_ = SharedStringImpl::create(".lazy");

        const char* fname = CHKPTR(filename())->c_str();
        auto_ptr<ELF::Binary> binary = get_symbol_tables_binary(fname);

        RefPtr<SymbolTable> head;
        ZObjectScope scope;

        head = read_tables(process(&scope),
                           filename(),
                           *binary,
                           *events_,
                           addr_,
                           upper_,
                           false /* don't call SymbolEvents::on_done() */);
        if (head)
        {
            next_ = head;
            addr_ = head->addr();
            adjust_ = head->adjustment();

            upper_ = head->upper();

            if (!filename()->is_equal(binary->name().c_str()))
            {
                set_realname(shared_string(binary->name()));
            }

            // the dust has settled: call the events observer's
            // on_done method
            for (; head; head = head->next())
            {
                events_->on_done(*head);
            }
            return true;
        }
    }

    return false;
}



/**
 * Lookup the closest symbol in the table to the given
 * virtual memory address.
 */
Symbol* SymbolTableImpl::lookup_symbol(addr_t addr) const
{
    if (is_lazy_stub())
    {
        return NULL;
    }

    Symbol* sym = 0; // result
    sortByAddr_->wait_for_completion();

    if (addr < this->addr() || symbols_.empty())
    {
        return NULL; // the address does not belong to this table
    }
    // lookup cached results
    SymHash::const_iterator k = symHash_.find(addr);
    if (k != symHash_.end())
    {
        return k->second.get();
    }

    // The method expects a virtual mem address, which it
    // translates down to a symbol table value, by subtracting
    // the base address where mapped into memory.
    addr = get_relative_addr(*this, addr);

    //
    // do the lookup proper, using a binary search:
    //
    SymAddrList::const_iterator i = lower_bound(symbols_.begin(),
                                                symbols_.end(),
                                                addr,
                                                CompareAddr());

    if ((i == symbols_.end()) || ((*i)->value() != addr))
    {
        if (i == symbols_.begin())
        {
            return 0;
        }
        --i;
        /*if (addr >= upper_)
        {
            IF_DEBUG
            (
                clog << __func__ << ": warning: " << hex << addr;
                clog << " is above upper limit: " << upper_ << dec << endl
            );
        }*/
        assert(*i);
        sym = new OffsetSymbolImpl(**i, addr - (*i)->value());
    }
    else
    {
        sym = i->get();
        assert(sym);
    }
    if (sym) // cache the result
    {
        symHash_[sym->addr()] = sym;
    }
    return sym;
}



/**
 * Enumerate all addresses associated with a given source
 * filename and line number (i.e. enumerate the addresses
 * of instructions generated for the given source line).
 */
size_t SymbolTableImpl::enum_addresses_by_line (
    SharedString*           file,
    size_t                  line,
    EnumCallback<addr_t>*   observer
    ) const
{
    if (is_lazy_stub())
    {
        CHECK_LAZY_LOAD();
        if (SymbolTable* table = next())
        {
            return events_->line_to_addr(*table, file, line, observer);
        }
        return 0;
    }
    return events_->line_to_addr(*this, file, line, observer);
}



typedef SymbolTableImpl::SymbolList::const_iterator Iterator;
typedef std::pair<Iterator, Iterator> SymbolsRange;


template<typename T> 
static void lookup_range(
    SharedString*   filename,
    const char*     name,
    const T&        table,
    SymbolsRange&   range /* out */)
{
    typedef typename T::predicate Pred;

    assert(name);
    if (!name)
    {
        return;
    }
    range.first = lower_bound(table.begin(), table.end(), name, Pred());

    if (range.first != table.end())
    {
        string upper = name;
        const char* sname = Pred::symbol_name(*range.first);

        if (strncmp(name, sname, upper.length()) != 0)
        {
            range.first = range.second;
            return;
        }

        assert (!upper.empty());
        ++(upper[upper.size() - 1]);

        range.second = lower_bound(range.first, table.end(), upper.c_str(), Pred());
    }
}


/**
 * Find a range of symbols that match the specified name
 */
template<typename T>
static size_t lookup_name (
    SharedString*               filename,
    const char*                 name,
    const T&                    table,
    EnumCallback<Symbol*>*      callback,
    SymbolTable::LookupMode     mode)
{
    typedef typename T::predicate Pred;

    SymbolsRange range(table.end(), table.end());

    if (mode & SymbolTable::LKUP_RANGE)
    {
        lookup_range(filename, name, table, range);
    }
    else
    {
        range = equal_range(table.begin(), table.end(), name, Pred());
    }

    if (callback)
    {
        for (Iterator i = range.first; i != range.second; ++i)
        {
            callback->notify(i->get());
        }
    }
    return distance(range.first, range.second);
}



void SymbolTableImpl::enum_all_symbols(EnumCallback<Symbol*>* cb) const
{
    assert(cb);
    if (cb)
    {
        for (auto i = symbols_.begin(); i != symbols_.end(); ++i)
        {
            cb->notify(i->get());
        }
    }
}



size_t SymbolTableImpl::enum_symbols (
    const char*             name,
    EnumCallback<Symbol*>*  cb,
    LookupMode              mode
    ) const
{
    if (is_lazy_stub())
    {
        return 0;
    }
    if (mode & LKUP_REGEX)
    {
        //
        //todo: support looking up symbols by regex
        //
        throw runtime_error(__func__ + string(": REGEX not impl."));
    }
    if (!addr_ && (mode & LKUP_UNMAPPED) == 0)
    {
        throw invalid_argument(__func__ + string(": LKUP_UNMAPPED"));
    }
    if (is_dynamic() && (mode & LKUP_DYNAMIC) == 0)
    {
        return 0;
    }
    wait_for_async_sort();

    if (size() == 0)
    {
        return 0; // table is empty, nothing to do
    }

    size_t count = 0;
    ////////////////////////////////////////////////////////////
    {   THREAD_SAFE();

        if (!name)
        {
            // no name given, enumerate all
            if (cb && (mode & LKUP_DEMANGLED))
            {
                SymbolsByDemangledName::const_iterator i =
                    symbolsByDemangledName_.begin();
                for (; i != symbolsByDemangledName_.end(); ++i)
                {
                    cb->notify(i->get());
                }
            }
            else
            {
                if (cb)
                {
                    enum_all_symbols(cb);
                }
                count = symbols_.size();
            }
        }
        else
        {
            // lookup up symbols that match the given name
            count = lookup_name(filename(), name, symbolsByName_, cb, mode);

            if ((mode & LKUP_ISMANGLED) == 0)
            {
                count += lookup_name(filename(),
                                     name,
                                     symbolsByDemangledName_,
                                     cb,
                                     mode);
            }
        }
    }
    return count;
}



SymbolTable* SymbolTableImpl::next() const
{
    if (!next_ && !is_loaded())
    {
        lazy_load(__func__, true);
    }
    return next_.get();
}



SymbolTableImpl* SymbolTableImpl::next_impl() const
{
    return interface_cast<SymbolTableImpl*>(next_.get());
}



SymbolTableImpl::Ptr SymbolTableImpl::read_tables
(
    Process*            proc,
    RefPtr<SharedString>fname,
    const ELF::Binary&  binary,
    SymbolTableEvents&  events,
    addr_t              vaddr,
    addr_t              upper,
    bool                notifyOnDone
)
{
    assert(fname);

    if (!binary.check_format())
    {
        return NULL;
    }

    ElfW(Xword) size = 0;
    ElfW(Addr) laddr = binary.get_load_addr(&size);

    if (!upper)
    {
        upper = vaddr + size;
    }
    const ELF::List<ELF::SymbolTable>& tables = binary.symbol_tables();

    ELF::List<ELF::SymbolTable>::const_iterator i(tables.begin());
    const ELF::List<ELF::SymbolTable>::const_iterator end(tables.end());

    Ptr prev, head;
    int8_t index = 0;
    for (i = tables.begin(); i != end; ++i)
    {
        assert(index < 127);
        Ptr symtab = new SymbolTableImpl(proc,
                                         fname,
                                         *i,
                                         events,
                                         vaddr,
                                         laddr,
                                         upper);
        symtab->index_ = index++;
        symtab->read(*i, notifyOnDone);

        if (!head)
        {
            head = symtab;
        }
        if (prev)
        {
            prev->next_ = symtab;
        }
        prev = symtab;
    }
    return head;
}



void SymbolTableImpl::sort_by_demangled_name() const
{
    assert(is_loaded()); // pre-condition

    symbolsByDemangledName_.sort();

    for (SymbolsByDemangledName::iterator i = symbolsByDemangledName_.begin();
         i != symbolsByDemangledName_.end();
         ++i)
    {
        (*i)->trim_memory();
    }
}


void SymbolTableImpl::wait_for_async_sort() const
{
    sortByAddr_->wait_for_completion();
    sortByName_->wait_for_completion();
    sortByDemangledName_->wait_for_completion();
}


auto_ptr<ELF::Binary> get_symbol_tables_binary(const char* path)
{
    auto_ptr<ELF::Binary> binary(new ELF::Binary(path));

    const ELF::List<ELF::SymbolTable>& tables = binary->symbol_tables();
    ELF::List<ELF::SymbolTable>::const_iterator i = tables.begin(),
                                              end = tables.end();

    bool haveStaticTable = false;
    for (; i != end; ++i)
    {
        if (i->header().type() != SHT_DYNSYM)
        {
            haveStaticTable = true;
            break;
        }
    }
    if (!haveStaticTable)
    {
        // look for symbols in an external file
        string link = ELF::debug_link(*binary);
        if (!link.empty())
        {
            binary.reset(new ELF::Binary(link.c_str()));
        }
    }
    assert(binary.get()); // post-condition
    return binary;
}


bool SymbolTableImpl::is_loaded() const
{
    return name_;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
