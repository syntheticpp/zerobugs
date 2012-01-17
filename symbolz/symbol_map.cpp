//
// $Id: symbol_map.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <sys/stat.h>
#include <sys/types.h>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "dharma/canonical_path.h"
#include "dharma/process_name.h"
#include "dharma/utility.h"
#include "elfz/public/headers.h"
#include "elfz/public/core_file.h"
#include "elfz/public/binary.h"
#include "elfz/public/dyn_lib_list.h"
#include "elfz/public/program_header.h"
#include "generic/lock.h"
#include "zdk/check_ptr.h"
#include "zdk/mutex.h"
#include "zdk/process.h"
#include "zdk/shared_string_impl.h"
#include "private/debug.h"
#include "private/link_data.h"
#include "private/symbol_map_impl.h"
#include "target/target.h"

#undef file_list
#undef lookup
#undef symbol_map

#define THREAD_SAFE Lock<Mutex> lock(mutex_);


using namespace std;

template<typename A>
static void debug_log(const char* fn, A a)
{
    if (debug_maps())
    {
        clog << fn << ": " << a << endl;
    }
}

template<typename A1, typename A2>
static void debug_log(const char* fn, A1 a1, A2 a2)
{
    if (debug_maps())
    {
        clog << fn << ": " << a1 << " " << a2 << endl;
    }
}


static size_t enum_symbols( RefPtr<SymbolTable>     tptr,
                            const char*             name,
                            EnumCallback<Symbol*>*  observer,
                            SymbolTable::LookupMode mode)
{
    size_t count = 0;

    for (SymbolTable* table = tptr.get(); table; table = table->next())
    {
        count += table->enum_symbols(name, observer, mode);
    }
    return count;
}


/*
static void
dump_symbol_map(ostream& out, SymbolMapImpl::SymbolTableMap& m)
{
    SymbolMapImpl::SymbolTableMap::iterator i = m.begin();
    for (; i != m.end(); ++i)
    {
        out << hex << i->first << ": ";
        for (SymbolTableImpl* table = i->second.get();
            table;
            table = table->next())
        {
            out << table->filename() << "/" << table->name();
            out << " " << table->addr() << "-" << table->upper();
        }
        out << endl;
    }
}
*/


SymbolMapImpl::~SymbolMapImpl() throw()
{
}


SymbolMapImpl::SymbolMapImpl(Process& process, SymbolTableEvents& events)
    : events_(events)
    , process_(&process)
    , arch_(EM_NONE)
    , loadAddr_(0)
    , scannedNeededTables_(false)
{
}


static auto_ptr<istream>
get_istream(const SymbolMapImpl& symbols, const string& filename)
{
    if (Process* proc = symbols.process())
    {
        if (RefPtr<Target> target = proc->target())
        {
            return target->get_ifstream(filename.c_str());
        }
    }
    return auto_ptr<istream>(new ifstream(filename.c_str()));
}


/**
 * Initial read, will be updated later when the special
 * dynamic linker rendez-vous breakpoint is hit
 *
 * @see /usr/include/link.h
 */
void SymbolMapImpl::read()
{
#if 0
    /* It would be really cool to get this to work, since it
     * would eliminate the need for reading the /proc/.../maps file,
     * which has an OS-dependent format
     */

    // read symbol info for the main executable,
    // will update shared object tables later
    // FIXME: does not work for attaching to an already running program

    ELF::Binary elf(process_->name());
    ElfW(Xword) mappedSize = 0;
    loadAddr_ = elf.get_load_addr(&mappedSize);

    string path = process_->name();
    events_.map_path(path);

    read_tables(loadAddr_, mappedSize, path.c_str(), map_);
#else
    //
    // read /proc/<pid>/map file and load symbol tables
    //
    // NOTE: this is Linux-specific: FreeBSD systems for example
    // use a different layout for /proc/<pid>/map
    //

    THREAD_SAFE;
    SymbolTableMap& tmp = map_;

    // open the maps file under the proc file-system
    ostringstream fname;
    fname << "/proc/" << process_->pid() << "/maps";
    auto_ptr<istream> fs(get_istream(*this, fname.str()));

    if (debug_maps())
    {
        system(("cat " + fname.str()).c_str());
    }
    vector<char> buf(4096);
    char dash;

    // there are 2 entries for one path, memorize the previous
    // path to prevent reading the symbol tables twice
    string prevPath;

    while (fs->getline(&buf[0], buf.size() - 1))
    {
        istringstream line(&buf[0]);
        addr_t base = 0;    // addr where file is mapped
        addr_t upper = 0;   // upper bound address

        line >> hex >> base >> dash >> hex >> upper;
        if (debug_maps())
        {
            clog << hex << base << dash << upper << dec << endl;
        }
        assert(upper >= base);

        // this should NEVER happen
        if (tmp.find(base) != tmp.end())
        {
            clog << "*** Warning: overlapped: ";
            clog << hex << base << dash << upper << dec << endl;
        }
        assert(tmp.find(base) == tmp.end());

        line.ignore(buf.size(), '/'); // skip to filepath
        line.unget();

        string path;
        // doesn't work for paths with white spaces in them:
        // if (!(line >> path)) continue;

        // ... so we are doing this instead:
        // assume the path is the last field on the line
        const ios::pos_type len = line.tellg();
        if (len < 0)
        {
            continue;
        }
        assert(static_cast<size_t>(len) < buf.size());
        const char* ptr = &buf[len];
        if (ptr == 0 || *ptr == 0)
        {
            continue;
        }
        path = ptr;
        if (prevPath != path)
        {
            prevPath = path;

            assert(!path.empty());
            // skip [heap], [stack], [vdso], [vsyscal] etc
            if (path[0] != '[' && path[0] != ']' && path[0] != ' ')
            {
                events_.map_path(this->process(), path);
                this->read_tables(base, 0, path, tmp);
            }
        }
    }
    mappedFiles_.reset(); // invalidate file_list
#endif
}


SymbolMapImpl::SymbolMapImpl
(
    Process& process,
    const ELF::CoreFile& core,
    SymbolTableEvents& events
)
    : events_(events)
    , process_(&process)
    , arch_(EM_NONE)
    , loadAddr_(0)
    , scannedNeededTables_(false)
{
    ELF::CoreFile::const_segment_iterator i = core.seg_begin();
    for (; i != core.seg_end(); ++i)
    {
        if (!i->second.filename.empty())
        {
            read_tables(i->first, i->second.msize, i->second.filename);
        }
        else if (i->first == get_load_addr())
        {
            read_tables(i->first, i->second.msize, process.name());
        }
    }
}


addr_t SymbolMapImpl::get_load_addr() const
{
    if (loadAddr_ == 0)
    {
        if (const char* name = process()->name())
        {
            string path(name);
            events_.map_path(process(), path);
            ELF::Binary elf(path.c_str());
            loadAddr_ = elf.get_load_addr();
        }
    }
    return loadAddr_;
}


SymbolTable* SymbolMapImpl::symbol_table_list(const char* path) const
{
    assert(path);

    THREAD_SAFE;
    SymbolTable* table = 0;

    SymbolTableMap::const_iterator i = map_.begin();
    for (; i != map_.end(); ++i)
    {
        if (i->second.get() && i->second->filename()->is_equal(path))
        {
            table = i->second.get();
            break;
        }
    }
    return table;
}

// todo: move this typedef to the ZDK
typedef EnumCallback<SymbolTable*> SymbolTableCallback;


size_t SymbolMapImpl::enum_symbol_tables(SymbolTableCallback* callback) const
{
    size_t result = 0;
    SymbolTableMap tmp;
    {
        THREAD_SAFE;
        tmp = map_;
    }
    for (SymbolTableMap::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
    {
        assert(i->second);
        if (callback)
        {
            callback->notify(i->second.get());
        }
        ++result;
    }
    return result;
}


/**
 * Iterate through tables that correspond to dynamic libs
 * that are not loaded in memory yet, but are specified in
 * the DT_NEEDED section (or have been added explicitly by
 * the user).
 */
size_t SymbolMapImpl::enum_needed_tables(SymbolTableCallback* callback) const
{
    THREAD_SAFE;

    if (Process* proc = process())
    {
        if (proc->origin() == ORIGIN_CORE)
        {
            IF_DEBUG(clog << __func__ << ": core file.\n");
            return 0;
        }
    }
    if (!scannedNeededTables_)
    {
        scan_needed_tables(); // populate the tables
        assert(scannedNeededTables_);
    }
    SymbolTableGroup::const_iterator i = neededTables_.begin();
    for (; i != neededTables_.end(); ++i)
    {
        if (is_mapped(i->first.c_str()))
        {
            continue;
        }

        if (callback)
        {
            callback->notify(i->second.get());
        }
    }
    return neededTables_.size();
}


bool SymbolMapImpl::is_needed(const string& filename) const
{
    bool result = neededTables_.find( filename ) != neededTables_.end();
    return result;
}


/**
 * Canonicalize filename and lookup table.
 */
bool SymbolMapImpl::is_mapped(const char* filename) const
{
    bool result = false;
    assert(filename);
    const string path = canonical_path(filename);

    SymbolTableMap::const_iterator i = map_.begin();
    for (; i != map_.end(); ++i)
    {
        if (i->second->filename()->is_equal(path.c_str()))
        {
            result = true;
            break;
        }
    }

    return result;
}


bool SymbolMapImpl::add_module_internal(const char* filename) const
{
    assert(filename);
    THREAD_SAFE;

    const string path(canonical_path(filename));

    if (is_needed(path))
    {
        return false;   // already scanned
    }

    scan_needed_tables(path.c_str());
    return true;
}


RefPtr<SymbolTable> SymbolMapImpl::read_tables(
    addr_t          addr,
    size_t          size,
    const string&   path)
{
    THREAD_SAFE;
    RefPtr<SymbolTable> symtbl = read_tables(addr, size, path, map_);
    if (symtbl)
    {
        mappedFiles_.reset(); // invalidate file_list
    }
    return symtbl;
}


/**
 * Read all symbol tables, static and dynamic, for the object
 * mapped at address ADDR
 */
RefPtr<SymbolTable> SymbolMapImpl::read_tables(
    addr_t          addr,
    size_t          size,
    const string&   path,
    SymbolTableMap& tableMap)
{
    addr_t loadAddr = 0;
    auto_ptr<ELF::Binary> binary;

    try
    {
        IF_DEBUG(clog << __func__ << ": " << path << endl);
        binary = get_symbol_tables_binary(path.c_str());

        if (!CHKPTR(binary)->check_format())
        {
            return 0; // ignore non-ELF files
        }
        ElfW(Xword) memsz = 0; // size in memory
        loadAddr = binary->get_load_addr(&memsz);

        if (size == 0)
        {
            // round up to page size
            if (ElfW(Xword) mod = (memsz % getpagesize()))
            {
                memsz += getpagesize() - mod;
            }
            size = memsz;
        }
    }
    catch (const exception& e)
    {
        IF_DEBUG(clog << __func__ << ": " << e.what() << endl);
        // silently ignore files that can't be opened
        return 0;
    }

    if (addr == 0)
    {
        addr = loadAddr;
    }
    const addr_t upper = addr + size;

    RefPtr<SymbolTable> symtbl;
    if (events_.use_lazy_loading())
    {
        // create a .lazy "stub"
        symtbl = new SymbolTableImpl(process(), path.c_str(), events_, addr, upper);
    }
    else
    {
        symtbl = SymbolTableImpl::read_tables(
            process(),
            shared_string(path),
            *binary,
            events_,
            addr,
            upper,
            false /* do not call SymbolTableEvents::on_done */);
    }

    if (symtbl)
    {
        assert(symtbl->addr() == addr);

        if (tableMap.insert(make_pair(addr, symtbl)).second)
        {
            // Notify the events observer that the table was loaded;
            // NOTE: this is done only AFTER the table is inserted into
            // the map, so that lookup_table will find this table if
            // called from SymbolTableEvents::on_done;
            // NOTE: call on_done here even if the table is not truly
            // loaded, the on_done implementation needs to call is_loaded

            if (!symtbl->is_loaded())
            {
                // next() has the side-effect of fully loading the table,
                // call on_done for the head only, so that saved
                // breakpoints get the opportunity to be restored

                events_.on_done(*symtbl);
            }
            else
            {
                for (SymbolTable* t = symtbl.get(); t; t = t->next())
                {
                    events_.on_done(*t);
                }
            }
        }
    }
    assert(!symtbl || symtbl->ref_count() > 1);
    assert(lookup_table(addr) == symtbl.get());

    return symtbl;
}


Symbol* SymbolMapImpl::lookup_symbol(addr_t addr) const
{
    THREAD_SAFE;
    RefPtr<Symbol> sym; // result

    // locate the binary file where given address maps to.
    SymbolTable* table = lookup_table(addr);
    if (!table)
    {
        sym = vdso_lookup(addr);
    }
 /*
    else if (CHKPTR(table)->upper() <= addr)
    {
        assert(table->filename());

        if (RefPtr<Symbol> tmp = vdso_lookup(addr))
        {
            sym.swap(tmp);
        }
    }
  */
    if (!sym)
    {
        for (RefPtr<SymbolTable> p = table; p; p = p->next())
        {
            assert(table->filename());

            if (RefPtr<Symbol> tmp = p->lookup_symbol(addr))
            {
                if (!sym || tmp->offset() < sym->offset())
                {
                    sym = tmp;
                }
            }
        }
    }
    return sym.detach();
}


RefPtr<Symbol> SymbolMapImpl::vdso_lookup(addr_t addr) const
{
    RefPtr<Symbol> sym;

    if (!vdsoTables_)
    {
        vdsoTables_.reset(process_->vdso_symbol_tables());
    }
    if (vdsoTables_)
    {
        if ((addr >= vdsoTables_->addr()) && (addr < vdsoTables_->upper()))
        {
            sym = vdsoTables_->lookup_symbol(addr);
        }
    }
    else
    {
        // clog << "***** No VDSO symbol table ******" << endl;
    }
    return sym;
}


SymbolTable* SymbolMapImpl::lookup_table(addr_t addr) const
{
    THREAD_SAFE;

    // Locate the binary file where given address maps to.
    SymbolTableMap::const_iterator i = map_.lower_bound(addr);
    if ((i == map_.end()) || (i->first != addr))
    {
        if (i == map_.begin())
        {
            return NULL;
        }
        --i;
    }
    return i->second.get();
}


void SymbolMapImpl::ensure_tables_loaded()
{
    for (SymbolTableMap::iterator i = map_.begin(); i != map_.end(); ++i)
    {
        i->second->ensure_loaded();
    }
}


size_t SymbolMapImpl::enum_symbols (
    const char*             name,
    EnumCallback<Symbol*>*  observer,
    SymbolTable::LookupMode mode)
{
    THREAD_SAFE;
    size_t count = 0;
    SymbolTableMap::iterator i = map_.begin();
    //
    // hack: if the name to lookup is "main", it is very
    // likely to be found in the image of the executable
    // rather than in some .so
    //
    if (name && strcmp(name, "main") == 0)
    {
        for (; i != map_.end(); ++i)
        {
            RefPtr<SymbolTable> table = i->second;
            RefPtr<SharedString> fname = table->filename();

            if (fname && process_ && fname->is_equal(process_->name()))
            {
                count = ::enum_symbols(table, name, observer, mode);
                if (count)
                {
                    return count;
                }
            }
        }
    } ///// end of "main" hack

    for (i = map_.begin(); i != map_.end(); ++i)
    {
        count += ::enum_symbols(i->second, name, observer, mode);
    }
    if (mode & SymbolTable::LKUP_UNMAPPED)
    {
        //
        // enumerate tables for shared objects that are not
        // loaded yet, but are listed as DT_NEEDED in the ELF
        // header of the executable, or of any other shared object
        //
        count += enum_needed_symbols(name, observer, mode);
    }
    return count;
}


/**
 * Discard modules that are in the  map but not in tmp
 * (because they have been unloaded).
 */
static void discard_unused (
    addr_t loadAddr,
    const set<addr_t>& tmp,
    SymbolMapImpl::SymbolTableMap& map)
{
    SymbolMapImpl::SymbolTableMap::iterator i = map.begin();
    for (; i != map.end(); )
    {
        assert(i->second.get());

        if (tmp.find(i->first) == tmp.end())
        {
            assert(i->first != loadAddr);
            if (i->second)
            {
                IF_DEBUG
                (
                    clog << __func__ << ": discarding: ";
                    clog << i->second->filename() << endl;
                )
            }
            map.erase(i++);
        }
        else
        {
            ++i;
        }
    }
}


void SymbolMapImpl::update(const LinkData* linkData)
{
    set<addr_t> tmp;

    THREAD_SAFE;
    SymbolTableMap& map = map_;

    for (; linkData; linkData = linkData->next())
    {
        const char* filename = CHKPTR(linkData->filename())->c_str();
        if (*CHKPTR(filename) == 0) // skip empty paths
        {
            tmp.insert(get_load_addr());
            continue;
        }
        string path = filename;
        if (!events_.map_path(process(), path))
        {
            path = canonical_path(filename);
        }
        if (Process* proc = process())
        {
            ensure_abs_lib_path(proc->environment(), path);
        }
        ELF::Binary binary(path.c_str());

        ElfW(Xword) mappedSize = 0;
        const addr_t elfLoadAddr = binary.get_load_addr(&mappedSize);
        const addr_t loadAddr = linkData->addr() + elfLoadAddr;

        debug_log(__func__, (void*)loadAddr, path);
        tmp.insert(loadAddr);
        //
        // have already loaded a shared library at this address?
        //
        SymbolTableMap::iterator i = map.find(loadAddr);
        if (i != map.end())
        {
            RefPtr<SymbolTable> table = i->second;
            assert(table);

            //same library?
            if (table->filename()->is_equal(path.c_str()))
            {
                assert(events_.use_lazy_loading()
                    || table->addr() == loadAddr);

                table->enum_deferred_breakpoints(&events_);
            }
            else // nope, not same lib
            {
                IF_DEBUG
                (
                    clog << __func__ << ": replacing ";
                    clog << table->filename() << " with " << path << endl;
                )
                map.erase(i);
                i = map.end();
            }
        }
        if (i == map.end())
        {
            bool mapped = map_library(map, path, loadAddr, elfLoadAddr, mappedSize);
            if (!mapped)
            {
                RefPtr<SymbolTable> tbl = read_tables(loadAddr, mappedSize, path, map);
                if (tbl)
                {
                    assert(events_.use_lazy_loading() || tbl->addr() == loadAddr);
                }
                else
                {
                    IF_DEBUG(clog << __func__ << " null: " << hex << loadAddr << dec << endl);
                }
            }
        }
    }
    discard_unused(get_load_addr(), tmp, map);

    mappedFiles_.reset();  // invalidate file_list
}


bool SymbolMapImpl::map_library (
    SymbolTableMap& map,
    const string&   path,
    addr_t          loadAddr,
    addr_t          elfLoadAddr,
    size_t          size)
{
    SymbolTableGroup::iterator u = neededTables_.begin();
    for (; u != neededTables_.end(); ++u)
    {
        if (u->first == path)
        {
            debug_log(__func__, ": mapping " + path + " at ", (void*)loadAddr);

            RefPtr<SymbolTableBase> tbl = u->second;
            tbl->set_addr(loadAddr, elfLoadAddr);
            tbl->set_upper(loadAddr + size);
            map.insert(make_pair(loadAddr, tbl));
            IF_DEBUG( clog << "MAPPED " << hex << loadAddr << dec << ": " << path << endl );

            neededTables_.erase(u);
            tbl->enum_deferred_breakpoints(&events_);
            events_.on_done(*tbl);
            return true;
        }
    }
    debug_log(__func__, "not found: " + path);
    return false;
}


SymbolMap::LinkData* SymbolMapImpl::file_list(RefTracker*) const
{
    THREAD_SAFE;

    if (!mappedFiles_)
    {
        RefPtr<LinkDataImpl> head;
        RefPtr<LinkDataImpl> tail;

        SymbolTableMap::const_iterator i = map_.begin();
        for (; i != map_.end(); ++i)
        {
            RefPtr<LinkDataImpl> node = new LinkDataImpl(i->second);
            assert(node->addr() == i->first);

            if (!head)
            {
                head = node;
            }
            if (tail)
            {
                tail->set_next(node);
            }
            tail = node;
        }
        mappedFiles_ = head;
    }
    return mappedFiles_.get();
}


/**
 * Iterate through dynamic libraries that are specified in the
 * DT_NEEDED section, but are not loaded into memory yet.
 * Lookup symbol by name. Return count of matching symbols.
 */
size_t SymbolMapImpl::enum_needed_symbols (
    const char*             name,
    EnumCallback<Symbol*>*  observer,
    SymbolTable::LookupMode mode)
{
    THREAD_SAFE;
    size_t count = 0;

    if (!scannedNeededTables_)
    {
        scan_needed_tables();
        assert(scannedNeededTables_);
    }

    SymbolTableGroup::const_iterator i = neededTables_.begin();
    for (; i != neededTables_.end(); ++i)
    {
        // each entry is the head of a linked list of tables;
        // each entry corresponds to a binary file
        RefPtr<SymbolTableBase> table = i->second;
        if (is_mapped(table->filename()->c_str()))
        {
            continue;
        }
        count += ::enum_symbols(table, name, observer, mode);
    }
    return count;
}


// NOTE: looks weird for this method to be const. 
// It is OK, it updates mutable stuff, and it HAS to be const
// as it is being called from enum_needed_tables, which the 
// ZDK interface defines as const (for better or worse)

void SymbolMapImpl::add_needed_tables(const char* moduleFileName) const
{
    assert(moduleFileName);
    const string path(moduleFileName);

    if (!is_needed(path) && !is_mapped(moduleFileName))
    {
        if (arch_ == EM_NONE && process())
        {
            ELF::Binary bin( process()->name() );
            arch_ = bin.header().machine();
        }

        // reject shared object of different architecture
        try
        {
            ELF::Binary lib(path.c_str());

            const long arch = lib.header().machine();

            if (arch_ == EM_NONE)
            {
                // perhaps not a fantastic idea?
                // arch_ = arch;
            }
            else if (arch_ != arch)
            {
                IF_DEBUG(clog << path << ": skipping architecture " << arch << endl);
                return;
            }
        }
        catch (const exception& e)
        {
            clog << path << ": " << e.what() << endl;
        }

        RefPtr<SymbolTableBase> table(
            new SymbolTableImpl(process(), moduleFileName, events_, 0, 0) );
        neededTables_.insert( make_pair(path, table) );
    }
}


CLASS Mapper : public PathMapper
{
    SymbolTableEvents&  events_;
    RefPtr<Process>     proc_;

public:
    Mapper(SymbolTableEvents& events, Process* proc)
        : events_(events)
        , proc_(proc)
    { }

    void apply(string& path) const
    {
        events_.map_path(proc_.get(), path);
    }
};


void SymbolMapImpl::scan_needed_tables(const string& path) const
{
    string fileName(path);
    const char* const* env = NULL;

    if (Process* proc = process())
    {
        env = proc->environment();
    }
    events_.map_path(process(), fileName);
    //
    // build a list of dependencies for the specified file
    //
    Mapper* mapper = new Mapper(events_, process());
    if (!dynLibs_.get())
    {
        dynLibs_.reset(new DynLibList(fileName.c_str(), env, mapper));
    }

    DynLibList::const_iterator j = dynLibs_->begin();
    for (; j != dynLibs_->end(); ++j)
    {
        add_needed_tables(j->c_str());
    }
    add_needed_tables(fileName.c_str());
}


void SymbolMapImpl::scan_needed_tables() const
{
    THREAD_SAFE;
    assert (!scannedNeededTables_);

    scan_needed_tables(process_->name());
    scannedNeededTables_ = true;
}


/**
 * Read the symbol map for a core file.
 */
RefPtr<SymbolMap>
read_symbols_from_core_dump(
    Process& process,
    const ELF::CoreFile& coreFile,
    SymbolTableEvents& events)
{
    return new SymbolMapImpl(process, coreFile, events);
}


/**
 * Read the symbol map of a running program.
 */
RefPtr<SymbolMap>
read_symbols_from_process(Process& process, SymbolTableEvents& events)
{
    RefPtr<SymbolMapImpl> symbols(new SymbolMapImpl(process, events));

    symbols->read();
    return symbols;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
