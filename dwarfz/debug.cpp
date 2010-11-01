//
// $Id: debug.cpp 715 2010-10-17 21:43:59Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include "generic/lock.h"
#include "zdk/check_ptr.h"
#include "zdk/mutex.h"
#include "zdk/shared_string_impl.h"
#include "zdk/string_cache.h"
#include "abi.h"
#include "canonical_path.h"
#include "compile_unit.h"
#include "decorated_type.h"
#include "datum.h"
#include "error.h"
#include "function.h"
#include "global.h"
#include "parameter.h"
#include "unwind.h"
#include "var.h"
#include "variable.h"
#include "private/cache.h"
#include "private/factory.h"
#include "private/log.h"
#include "impl.h"
#include "debug.h"
#include "log.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"

using namespace std;
using namespace boost;
using namespace Dwarf;


/**
 * map opaque Dwarf_Debug handles to Dwarf::Debug objects
 */
CLASS DebugMap : private std::map<Dwarf_Debug, Debug*>
{
    mutable Mutex mutex_;

public:
    static DebugMap* instance_;

    typedef map<Dwarf_Debug, Debug*> base_type;
    typedef base_type::const_iterator const_iterator;
    typedef base_type::iterator iterator;

    DebugMap()
    {
        assert(!instance_);
        instance_ = this;
    }

    ~DebugMap() { instance_ = 0; }

    static DebugMap& instance()
    {
        if (!instance_)
        {
            throw logic_error("DebugMap::instance");
        }
        return *instance_;
    }

    const_iterator find(Dwarf_Debug dbg) const
    {
        Lock<Mutex> lock(mutex_);
        return base_type::find(dbg);
    }

    const_iterator end() const { return base_type::end(); }

    void erase(Dwarf_Debug dbg)
    {
        Lock<Mutex> lock(mutex_);
        base_type::erase(dbg);
    }

    template<typename T>
    void erase(T i)
    {
        assert(i != end());

        Lock<Mutex> lock(mutex_);
        base_type::erase(i);
    }

    template<typename T>
    void insert(T v)
    {
        Lock<Mutex> lock(mutex_);
        base_type::insert(v);
    }
};


DebugMap* DebugMap::instance_ = NULL;

namespace Dwarf
{
    ZDK_LOCAL DebugMap* __init__ = new DebugMap();
}


/**
 * Maintains (srcfile,linenum) <-> addr mapping
 */
CLASS Debug::SrcLineMap : public SrcLineEvents
{
    typedef multimap<size_t, Dwarf_Addr> AddrByLine;
    typedef ext::hash_map<RefPtr<SharedString>, AddrByLine> AddrByFile;

    AddrByLine addrByLine_;
    AddrByFile addrByFile_;

    const Debug& dbg_;
    mutable bool cancelled_;

    //vector<RefPtr<SharedString> > fileTable_;
    double prevPercent_;

    RefPtr<SharedString> unitFileName_;
    mutable Mutex mutex_;

public:
    ////////////////////////////////////////////////////////////
    explicit SrcLineMap(const Debug& dbg)
        : dbg_(dbg)
        , cancelled_(false)
        , prevPercent_(.0)
    {
        cache_line_info();
    }

    ////////////////////////////////////////////////////////////
    SrcLineMap(const Debug& dbg, CompileUnit& unit)
        : dbg_(dbg)
        , cancelled_(false)
        , prevPercent_(.0)
        , unitFileName_(unit.full_path())
    {
        if (unit.cache_srclines(this))
        {
            on_done();
        }
        else
        {
           cancelled_ = true;
        }
        print_reports(clog);
    }

    ////////////////////////////////////////////////////////////
    bool cancelled() const { return cancelled_; }

  /*
    RefPtr<SharedString> filename_by_index(size_t n) const
    {
        RefPtr<SharedString> fname;

        if (n < fileTable_.size())
        {
            fname = fileTable_[n];
        }
        return fname;
    } */

    ////////////////////////////////////////////////////////////
    bool matches(const char* filename) const
    {
        //
        // empty unit filename means we built it for
        // the entire module -> match ok
        //
        return unitFileName_.is_null()
            || unitFileName_->length() == 0
            || strcmp(filename, unitFileName_->c_str()) == 0;
    }

private:
    void on_done() { }

    ////////////////////////////////////////////////////////////
    void cache_line_info()
    {
        cancelled_ = false;
        prevPercent_ = .0;

        const UnitList& units = dbg_.units();

        UnitList::const_iterator i = units.begin();
        const UnitList::const_iterator end = units.end();

        for (; i != end; ++i)
        {
            if (!(*i)->cache_srclines(this))
            {
                cancelled_ = true;
                break;
            }
        }
        print_reports(clog);
    }

    ////////////////////////////////////////////////////////////
    void print_reports(ostream& out) const
    {
#if 0 //DEBUG
        size_t nCount = 0;
        for (AddrByFile::const_iterator i = addrByFile_.begin();
             i != addrByFile_.end();
             ++i)
        {
            nCount += i->second.size();
        }
        out << dbg_.filename() << " addrByLine: "
            << addrByLine_.size() << " "
            << " addrByFile: " << nCount << "/"
            << addrByFile_.size() << " " << endl;
#endif
    }

    ////////////////////////////////////////////////////////////
    bool on_srcline(SharedString*   file,
                    Dwarf_Unsigned  line,
                    Dwarf_Addr      addr)
    {
        if (file)
        {
            assert(*file->c_str() == '/');
            RefPtr<SharedString> path(file);

            Lock<Mutex> lock(mutex_);
          /*
            if (addrByFile_.find(path) == addrByFile_.end())
            {
                fileTable_.push_back(path);
            } */
            AddrByFile::iterator i = addrByFile_.find(path);
            if (i != addrByFile_.end())
            {
                path = i->first;
            }
            AddrByLine& addrByLine = addrByFile_[path];
            addrByLine.insert(make_pair(line, addr));
        }
        return true;
    }

    ////////////////////////////////////////////////////////////
    bool line_to_addr(const AddrByLine& m,
                      SharedString* file,
                      size_t line,
                      SrcLineEvents& events) const
    {
        if (m.empty())
        {
            return false;
        }
        AddrByLine::const_iterator j = m.lower_bound(line);
        if (j == m.end())
        {
            assert(j != m.begin());
            --j;
        }
        const size_t lineNum = j->first;

        for (; j != m.end() && j->first == lineNum; ++j)
        {
            const Dwarf_Addr addr = j->second;

            if (!events.on_srcline(file, lineNum, addr))
            {
                cancelled_ = true;
                return false;
            }
        }
        return true;
    }

public:
    /**
     * Given a source file name and a line number, lookup the
     * nearest address(es); for each match call the observer
     * (SrcLineEvents::on_srcline)
     * @return true if completed successfully, false if the
     * operation got interrupted
     */
    bool line_to_addr(SharedString*  file,
                      size_t         line,
                      SrcLineEvents& events) const
    {
        assert(file);
        bool result = true;

        if (cancelled_)
        {
            result = false;
        }
        else if (file)
        {
            Lock<Mutex> lock(mutex_);

            AddrByFile::const_iterator i = addrByFile_.find(file);
            if (i != addrByFile_.end())
            {
                const AddrByLine& m = i->second;
                result = line_to_addr(m, file, line, events);
            }
        }
        return result;
    }
};


CLASS Debug::UnitMap : public map<Dwarf_Off, shared_ptr<CompileUnit> >
{
    // empty
};


////////////////////////////////////////////////////////////////
Debug* Debug::get_wrapper(Dwarf_Debug dbg)
{
    Debug* result = 0;

    DebugMap::const_iterator i = DebugMap::instance().find(dbg);
    if (i != DebugMap::instance().end())
    {
        result = i->second;
    }
    return result;
}


////////////////////////////////////////////////////////////////
DebugBase::DebugBase(const char* filename)
    : fd_(open(filename, O_RDONLY))
    , dbg_(NULL)
{
}


////////////////////////////////////////////////////////////////
DebugBase::~DebugBase() throw()
{
    if (dbg_)
    {
        DebugMap::instance().erase(dbg_);

        assert(dbg_);
        dwarf_finish(dbg_, 0);

        dbg_ = 0;
    }
}


////////////////////////////////////////////////////////////////
Debug::Debug
(
    const char* filename,
    const UnitHeadersCallback* callback
)
    : DebugBase(filename)
    , globalCache_(0)
    , srcLineMap_(0)
    , unitMap_(new UnitMap)
    , inode_(0)
    , unitHeadersCallback_(callback)
{
    if (filename)
    {
        filename_ = filename;
    }

    if (!fd_)
    {
        throw runtime_error(strerror(errno));
    }

    Dwarf_Error err = 0;
    int r = dwarf_init(fd_.get(), DW_DLC_READ, 0, 0, &dbg_, &err);
    if (r == DW_DLV_ERROR)
    {
        string errmsg("dwarf_init: ");
        if (filename)
        {
            errmsg += filename;
        }
        throw Error(errmsg.c_str(), dbg_, err);
    }
    if (dbg_)
    {
        DebugMap::instance().insert(make_pair(dbg_, this));
    }
}


////////////////////////////////////////////////////////////////
Debug::~Debug() throw()
{
    if (globalCache_)
    {
        delete globalCache_;
    }
    if (srcLineMap_)
    {
        delete srcLineMap_;
    }
    if (unitMap_)
    {
        delete unitMap_;
    }
}


////////////////////////////////////////////////////////////////
ino_t Debug::inode() const
{
    if (inode_ == 0)
    {
        struct stat st;
        while (fstat(fd_.get(), &st) < 0)
        {
            if (errno != EINTR)
            {
                throw runtime_error(
                    string(__func__) + ": " + strerror(errno));
            }
        }
        inode_ = st.st_ino;
    }
    return inode_;
}


////////////////////////////////////////////////////////////////
Dwarf::FunList Debug::lookup_global_funcs(const char* name) const
{
    vector<shared_ptr<Function> > funcs;

    if ((globalCache_ == 0) && dbg_)
    {
        globalCache_ = new Cache<Dwarf_Global>(dbg_);
    }
    if (globalCache_)
    {
        Cache<Dwarf_Global>::range r;
        if (name)
        {
            r = globalCache_->lookup(name);
        }
        else
        {
            r = globalCache_->all();
        }
        // todo: up to here, this code is duplicated in
        // lookup_global_data, the loop is also very
        // similar -- re-factor
        Cache<Dwarf_Global>::const_iterator i = r.first;
        for (; i != r.second; ++i)
        {
            Dwarf_Off off = i->second->die_offset();
            shared_ptr<Die> die = get_object(off);

            if (shared_ptr<Function> fp =
                    shared_dynamic_cast<Function>(die))
            {
                funcs.push_back(fp);
            }
        }
    }
    return funcs;
}


////////////////////////////////////////////////////////////////
const Dwarf::FunList& Debug::global_funcs() const
{
    if (!funcs_.get())
    {
        funcs_.reset(new FunList(lookup_global_funcs(NULL)));
    }
    assert(funcs_.get());
    return *funcs_;
}


////////////////////////////////////////////////////////////////
Debug::Data Debug::lookup_global_data(const char* name) const
{
    assert(name);
    Data result;

    if (globalCache_ == 0 && dbg_)
    {
        globalCache_ = new Cache<Dwarf_Global>(dbg_);
    }
    if (globalCache_)
    {
        Cache<Dwarf_Global>::range r;
        if (name)
        {
            r = globalCache_->lookup(name);
        }
        else
        {
            r = globalCache_->all();
        }
        Cache<Dwarf_Global>::const_iterator i = r.first;
        for (; i != r.second; ++i)
        {
            Dwarf_Off off = i->second->die_offset();
            shared_ptr<Die> die = get_object(off);

            if (shared_ptr<Datum> datum = shared_dynamic_cast<Datum>(die))
            {
                datum->set_global(i->second);
                result.push_back(datum);
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
shared_ptr<Function>
Debug::lookup_function(Dwarf_Addr addr, const char* linkage) const
{
    shared_ptr<Function> result;
    // first, find the compile-unit where the addr belongs
    if (shared_ptr<CompileUnit> unit = lookup_unit(addr))
    {
        // lookup function within the compile-unit
        result = unit->lookup_function(addr, linkage);
        if (!result)
        {
            unit->uncache_functions();
        }
    }
    else
    {
        log<debug>() << __func__ << ": unit not found for addr: "
                     << hex << addr << dec << "\n";
    }
    if (!result)
    {
        result = Utils::lookup_function(global_funcs(), addr, linkage);
    }
    return result;
}


////////////////////////////////////////////////////////////////
bool Debug::units_from_unit_headers() const
{
    vector<Dwarf_Unsigned> headers;
    if (unitHeadersCallback_)
    {
        unitHeadersCallback_->query_unit_headers(filename_, headers);
    }
    bool result = false;
    if (!headers.empty())
    {
        shared_ptr<CompileUnit> unit = CompileUnit::next_unit(dbg_, 0);
        if (unit)
        {
            CHKPTR(units_)->push_back(unit);
        }
        for (vector<Dwarf_Unsigned>::const_iterator u = headers.begin();
             u != headers.end();
             ++u)
        {
            unit = CompileUnit::next_unit(dbg_, *u);
            if (unit)
            {
                CHKPTR(units_)->push_back(unit);
            }
        }
        result = true;
    }
    return result;
}


////////////////////////////////////////////////////////////////
const Debug::UnitList& Debug::units() const
{
    Lock<Mutex> lock(mutex_);

    if (!units_.get())
    {
        units_.reset(new UnitList);

        if (units_from_unit_headers())
        {
            log<debug>(0) << filename() << ": "
                          << units_->size() << " compilation units\n";
        }
        else
        {
            //assert(!unitsByRange_.get());
            //unitsByRange_.reset(new UnitsByRange);
            List<CompileUnit> cuList(dbg_, 0);
            List<CompileUnit>::iterator i = cuList.begin();
            List<CompileUnit>::iterator end = cuList.end();

            for (; i != end; ++i)
            {
                assert(i);
                log<debug>(1)<< "=== " << i->name() << ": "
                             << hex << i->low_pc() << "-"
                             << i->high_pc() << dec << "\n";

                units_->push_back(i);

            #if 0
                // for now, assume that a compilation unit
                // produces a contiguous address range
                RangeEntry r(i->high_pc(), i);
                unitsByRange_->insert(make_pair(i->low_pc(), r));
            #endif
            }
        }
    }
    assert(units_.get());
    return *units_;
}


/**
 * Helper class for automatically de-allocating a list of
 * dwarf address ranges -- move it to its own file when it
 * matures
 */
CLASS AddressRanges
{
    AddressRanges(const AddressRanges&);
    AddressRanges& operator=(const AddressRanges&);

    Dwarf_Debug     dbg_;
    Dwarf_Arange*   ranges_;
    Dwarf_Signed    size_;

public:
    explicit AddressRanges(Dwarf_Debug dbg) : dbg_(dbg), ranges_(0), size_(0)
    {
        Dwarf_Error err = 0;
        if (dwarf_get_aranges(dbg_, &ranges_, &size_, &err) == DW_DLV_ERROR)
        {
            cerr << Error("dwarf_get_aranges", dbg_, err).what() << endl;
        }
    }

    ~AddressRanges()
    {
        for (Dwarf_Signed i = 0; i != size_; ++i)
        {
            dwarf_dealloc(dbg_, ranges_[i], DW_DLA_ARANGE);
        }
        dwarf_dealloc(dbg_, ranges_, DW_DLA_LIST);
    }

    Dwarf_Arange get_range(Dwarf_Addr addr) const
    {
        Dwarf_Arange arange = 0;
        Dwarf_Error  err = 0;

        if (dwarf_get_arange(
                ranges_, size_, addr, &arange, &err) != DW_DLV_OK)
        {
            dwarf_dealloc(dbg_, err, DW_DLA_ERROR);
            arange = 0;
        }
        return arange;
    }
};


////////////////////////////////////////////////////////////////
shared_ptr<CompileUnit>
Debug::lookup_unit_by_arange(Dwarf_Addr addr) const
{
    shared_ptr<CompileUnit> unit; // result

    AddressRanges ranges(dbg_);
    if (Dwarf_Arange arange = ranges.get_range(addr))
    {
        Dwarf_Error err = 0;
        Dwarf_Off   off = 0;
        Dwarf_Addr  start = 0;
        Dwarf_Unsigned len = 0;

        if (dwarf_get_arange_info(arange, &start, &len, &off, &err) == DW_DLV_OK)
        {
            log<debug>(1) << __func__ << ": off="
                          << hex << off << " start=" << start
                          << " end=" << start + len << dec
                          << " length=" << len << "\n";

            UnitMap::const_iterator i = unitMap_->find(off);

            /* this assert may actually fail, as per crash report
               from: markm11@charter.net, Wed, February 20, 2008 9:09 am */
            /* assert(i != unitMap_->end()); */

            if (i != unitMap_->end())
            {
                unit = i->second;
            }
        }
        else
        {
            log<error>() << Error(__func__, dbg_, err).what() << "\n";
        }
    }
    return unit;
}


////////////////////////////////////////////////////////////////
shared_ptr<CompileUnit>
Debug::lookup_unit(Dwarf_Addr addr) const
{
    shared_ptr<CompileUnit> unit;

    if (units().size())
    {
    #if 1
        assert(units_.get());

        bool byArange = false;

        UnitList::const_iterator ii = units_->begin();
        for (size_t j = 0; ii != units_->end(); ++ii, ++j)
        {
            const Dwarf_Addr lowPC = (*ii)->low_pc();
            const Dwarf_Addr highPC = (*ii)->high_pc();

            if ((lowPC == 0) || (highPC == 0))
            {
                byArange = true;
            }

            if (lowPC <= addr && highPC > addr)
            {
                unit = *ii;
                break;
            }
        }

        if (byArange && !unit)
        {
            unit = lookup_unit_by_arange(addr);
        }
    #else

        assert(unitsByRange_.get());

        UnitsByRange::iterator i = unitsByRange_->lower_bound(addr);
        if ((i == unitsByRange_->end()) || (i->first != addr))
        {
            if (i != unitsByRange_->begin())
            {
                --i;
            }
        }
        for (; (i != unitsByRange_->end()) && (i->first <= addr); ++i)
        {
            if (addr < i->second.upper_)
            {
                unit = i->second.unit_;
                break;
            }
        }
    /*
        if (!unit)
        {
            unit = lookup_unit_by_arange(addr);
        } */
    #endif
    }
    return unit;
}



////////////////////////////////////////////////////////////////
shared_ptr<CompileUnit>
Debug::lookup_unit(const char* fname) const
{
    shared_ptr<CompileUnit> unit; // result

    UnitList::const_iterator i = units().begin();
    for (; i != units().end(); ++i)
    {
        if (strcmp((*i)->full_path()->c_str(), fname) == 0)
        {
            unit = *i;
            break;
        }
    }
    return unit;
}



////////////////////////////////////////////////////////////////
shared_ptr<CompileUnit>
Debug::get_compile_unit(Dwarf_Die die,
                        Dwarf_Off nextUnitHdr)const
{
    shared_ptr<CompileUnit> unit;

    Dwarf_Off off = Die::offset(dbg_, die);

    UnitMap::iterator i = unitMap_->find(off);
    if (i != unitMap_->end())
    {
        unit = i->second;
    }
    else if (die)
    {
        unit.reset(new CompileUnit(dbg_, die, nextUnitHdr, strCache_));
        unitMap_->insert(i, make_pair(off, unit));
    }
    return unit;
}


////////////////////////////////////////////////////////////////
bool Debug::line_to_addr(
    SharedString*   file,
    size_t          line,
    SrcLineEvents&  events) const
{
    const char* const filename = CHKPTR(file)->c_str();

    if (!validate_line_map(filename))
    {
        if (shared_ptr<CompileUnit> unit = lookup_unit(filename))
        {
            srcLineMap_ = new SrcLineMap(*this, *unit);
        }
    }

    if (!validate_line_map(filename))
    {
        srcLineMap_ = new SrcLineMap(*this);
    }
    return srcLineMap_->line_to_addr(file, line, events);
}


////////////////////////////////////////////////////////////////
Dwarf_Addr Debug::next_line(
    const char* file,
    size_t      line,
    Dwarf_Addr  addr,
    size_t*     next) const
{
    if (shared_ptr<CompileUnit> unit = lookup_unit(addr))
    {
        return unit->next_line(file, line, addr, next);
    }

    return 0;
}


////////////////////////////////////////////////////////////////
shared_ptr<Die>
Debug::get_object(Dwarf_Off off, bool useSpec, bool indirect) const
{
    shared_ptr<Die> object;
    DieCache::iterator i = dieCache_.find(off);
    if (i != dieCache_.end())
    {
        object = i->second;
    }
    else
    {
        Dwarf_Die die = 0;
        Dwarf_Error err = 0;

        if (dwarf_offdie(dbg_, off, &die, &err) == DW_DLV_ERROR)
        {
            dwarf_dealloc(dbg_, err, DW_DLA_ERROR);
        }
        else
        {
            object = Factory::instance().create(dbg_, die);
        }
        if (object)
        {
            dieCache_.insert(make_pair(off, object));

            if (indirect)
            {
                if (shared_ptr<Die> tmp = object->check_indirect(useSpec))
                {
                    object = get_object(tmp->offset());
                }
            }
        }
    }
    return object;
}


////////////////////////////////////////////////////////////////
static bool
compare_types(const char* type, const char* name, size_t len)
{
    assert(name);

    if (type)
    {
        if (strcmp(type, name) == 0)
        {
            return true;
        }
        if (strlen(type) > len)
        {
            switch (type[len])
            {
            //case '*':
            //case '&':
            case ':':
                return strncmp(type, name, len) == 0;
            }
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
static shared_ptr<Type>
compare_types(shared_ptr<Type> type, const char* name, size_t length)
{
    assert(type);
    assert(name);

    for (; type;)
    {
        if (compare_types(type->name(), name, length))
        {
            return type;
        }

        DecoratedType* d =
            dynamic_cast<Dwarf::DecoratedType*>(type.get());
        if (!d)
        {
            break;
        }
        type = d->type();
    }
    return shared_ptr<Type>();
}


/**
 * Helper that looks up a type by name inside of a lexical block
 */
static shared_ptr<Type>
find_type(const Block& block, const char* name, size_t length)
{
    shared_ptr<Type> type;

    if (name && *name)
    {
        assert(length);
        const VarList& vars = block.variables();

        VarList::const_iterator v = vars.begin();
        VarList::const_iterator endv = vars.end();

        for (; v != endv; ++v)
        {
            if (!(*v)->type())
            {
                continue;
            }

            type = compare_types((*v)->type(), name, length);
            if (type)
            {
                break;
            }
            else
            {
                // descend into sub-blocks
                Dwarf::List<Block> subBlocks = block.blocks();
                Dwarf::List<Block>::const_iterator
                    i = subBlocks.begin(),
                    end = subBlocks.end();

                for (; i != end; ++i)
                {
                    type = find_type(*i, name, length);
                    if (type)
                    {
                        return type;
                    }
                }
            }
        }
    }
    return type;
}


////////////////////////////////////////////////////////////////
static shared_ptr<Type>
find_type_in_function(const Function& func, const char* name)
{
    shared_ptr<Type> type; // result
    if (name && *name)
    {
        const size_t length = strlen(name);

        typedef Function::ParamList PList;
        const PList& params = func.params();

        PList::const_iterator i = params.begin();
        PList::const_iterator end = params.end();

        for (; i != end && !type; ++i)
        {
            assert((*i)->type());
            type = compare_types((*i)->type(), name, length);
        }
        if (!type)
        {
            type = find_type(func, name, length);
        }
    }
    return type;
}


////////////////////////////////////////////////////////////////
shared_ptr<Type>
Dwarf::lookup_type( const FunList& funcs,
                    const char* name,
                    bool complete )
{
    shared_ptr<Type> type;

    FunList::const_iterator i = funcs.begin();
    FunList::const_iterator end = funcs.end();
    for (; i != end; ++i)
    {
        assert(*i);

        type = find_type_in_function(**i, name);
        if (type)
        {
            if (!complete || type->is_complete())
            {
                break;
            }
        }
    }
    return type;
}


/**
 * Given a type declaration, lookup the complete definition
 */
shared_ptr<Type>
Debug::lookup_type_by_decl(const Type& decl) const
{
    static const bool completeTypes = true;

    shared_ptr<Type> type =
        Dwarf::lookup_type(global_funcs(), decl.name(), completeTypes);

    if (!type)
    {
        const UnitList& units = this->units();
        UnitList::const_iterator i = units.begin(), end = units.end();

        for (i = units.begin(); i != end; ++i)
        {
            type = (*i)->lookup_type(decl);

            if (type && type->is_complete())
            {
                break;
            }
        }
    }

    if (type && (type->offset() == decl.offset()))
    {
        type.reset();
    }
    assert(!type || type->offset() != decl.offset());
    return type;
}


/**
 * Lookup a type, by name, in a list of compilation units
 */
static shared_ptr<Type>
lookup_type_in_units(const Debug::UnitList& units, const char* name)
{
    shared_ptr<Type> type;

    Debug::UnitList::const_iterator i = units.begin(),
        end = units.end();
    for (; i != end; ++i)
    {
        type = (*i)->lookup_type(name);
        if (type)
        {
            if (type->is_incomplete())
            {
                if (shared_ptr<Type> tmp = (*i)->lookup_type(*type))
                {
                    type.swap(tmp);
                }
            }
            break;
        }
    }
    return type;
}


////////////////////////////////////////////////////////////////
shared_ptr<Type> Debug::lookup_type(const char* name, int level) const
{
    shared_ptr<Type> type;

    {
        type = lookup_type_by_ctor(name);

        if (type && !type->is_complete())
        {
        #ifdef DEBUG
             clog << "*** Incomplete type in ctor: " << name;
             clog << ": " << type->name() << " ***\n";
        #endif
             type.reset();
        }
    }

    if (!type && level)
    {
        type = lookup_type_in_units(units(), name);
        if (!type && (level > 1))
        {
            log<debug>() << "looking up global funcs for type: " << name << "\n";
            type = Dwarf::lookup_type(global_funcs(), name);
        }
    }

    if (type && type->is_incomplete() /* && useExpensiveLookup */)
    {
        if (shared_ptr<Type> tmp = lookup_type_by_decl(*type))
        {
            assert(tmp->is_complete());
            type.swap(tmp);
        }
    }
    return type;
}


////////////////////////////////////////////////////////////////
string Debug::infer_ctor_name(const char* name)
{
    string ctor(name);

    // check for fully-qualified names
    size_t n = ctor.rfind("::");
    string prefix;
    if (n == string::npos)
    {
        n = 0;
    }
    else
    {
        size_t m = ctor.rfind("::", n - 1);
        if (m == string::npos)
        {
            prefix = ctor.substr(0, n);
        }
        else
        {
            prefix = ctor.substr(m + 2, n);
        }
        n += 2;
    }
    string unqualifiedName = ctor.substr(n);
    if (prefix != unqualifiedName)
    {
        ctor += "::" + unqualifiedName;
    }
    return ctor;
}


////////////////////////////////////////////////////////////////
shared_ptr<Type>
Debug::lookup_type_by_ctor(const string& ctor) const
{
    shared_ptr<Type> result;

    // look for constructor(s)
    vector<shared_ptr<Function> > funcs =
        lookup_global_funcs(ctor.c_str());

    // in case the compiler does not generate .debug_pubnames,
    // dive into each unit
    if (funcs.empty())
    {
#if 0 // this is an expensive operation -- I don't think that it
      // is needed for GCC, I have to double-check Intel Compiler

        UnitList::const_iterator i = units().begin();

        for (; !result && i != units().end(); ++i)
        {
            const FunList& funcs = (*i)->functions();

            FunList::const_iterator j = funcs.begin();
            const FunList::const_iterator end = funcs.end();

            for (; j != end; ++j)
            {
                if ((ctor == (*j)->name() || ctor == (*j)->name())
                    && !(*j)->params().empty())
                {
                    shared_ptr<Parameter> tp = *(*j)->params().begin();
                    result = tp->type();
                    break;
                }
            }
        }
#endif
    }
    // got at least one ctor?
    else if (!funcs.front()->params().empty())
    {
        shared_ptr<Parameter> tp = *funcs.front()->params().begin();
        result = tp->type();
    }
    if (result)
    {
        for (;;)
        {
            // expect const-pointer to type; remove decorators
            shared_ptr<DecoratedType> deco =
                shared_dynamic_cast<DecoratedType>(result);

            if (!deco)
            {
                break;
            }
            result = deco->type();
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
shared_ptr<Type> Debug::lookup_type_by_ctor(const char* name) const
{
    assert(name);

    string ctor = infer_ctor_name(name);
    return lookup_type_by_ctor(ctor);
}



////////////////////////////////////////////////////////////////
Dwarf_Addr
Debug::unwind_step(Dwarf_Addr pc, AddrOps& funcs, RegTable& regs)
{
    if (!unwind_)
    {
    #ifdef DEBUG
        log<debug>() << __func__ << ": " << filename() << "\n";
    #endif
        unwind_.reset(new Unwind(dbg_));
    }
    Dwarf_Addr addr = unwind_->step(pc, funcs, regs);
    return addr;
}


////////////////////////////////////////////////////////////////
bool Debug::validate_line_map(const char* filename) const
{
    if (srcLineMap_)
    {
        if (srcLineMap_->cancelled() || !srcLineMap_->matches(filename))
        {
            delete srcLineMap_;
            srcLineMap_ = 0;
        }
    }
    return srcLineMap_;
}



////////////////////////////////////////////////////////////////
void Debug::cache_object(Dwarf_Die die) const
{
    Dwarf_Off off = Die::offset(dbg_, die);

    Lock<Mutex> lock(mutex_);
    DieCache::const_iterator i = dieCache_.find(off);

    if (i == dieCache_.end())
    {
        if (shared_ptr<Die> object = Factory::instance().create(dbg_, die))
        {
            dieCache_.insert(make_pair(off, object));
        }
    }
    else
    {
        dwarf_dealloc(dbg_, die, DW_DLA_DIE);
    }
}


////////////////////////////////////////////////////////////////
bool Debug::empty() const
{
    Dwarf_Die die = 0;
    Dwarf_Error err = 0;

    bool isEmpty = (dwarf_siblingof(dbg_, NULL, &die, &err) != DW_DLV_OK
                 || die == NULL);

    if (die) dwarf_dealloc(dbg_, die, DW_DLA_DIE);
    if (err) dwarf_dealloc(dbg_, err, DW_DLA_ERROR);

    return isEmpty;
}


////////////////////////////////////////////////////////////////
void Debug::set_string_cache(StringCache* stringCache)
{
    strCache_ = stringCache;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
