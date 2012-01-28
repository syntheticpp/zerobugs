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
#include <functional>
#include "dharma/canonical_path.h"
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/export.h"
#include "zdk/shared_string_impl.h"
#include "zdk/zobject_scope.h"
#include "debugger_engine.h"
#include "dharma/environ.h"
#include "history.h"
#include "module.h"
#include "symbol_events.h"

using namespace std;


/********** Helpers **********/
namespace
{
    /**
     * Iterates thru all plugins, looking for those that
     * implement the DebuggerPlugin interface, and call
     * source_line() on them.
     */
    class ZDK_LOCAL AddrToLineHelper
        : public EnumCallback2<SharedString*, size_t>
    {
    public:
        AddrToLineHelper() : line_(0) {}
        virtual ~AddrToLineHelper() {}

        SharedString* source() const { return source_.get(); }

        size_t line() const { return line_; }

    protected:
        void notify(SharedString* src, size_t line)
        {
            source_ = src;
            line_ = line;
        }

    private:
        RefPtr<SharedString> source_;
        size_t line_;
    };


    /**
     * Functor for invoking on_table_init and on_table_done methods,
     * using the DebuggerEngine::for_each_plugin template method.
     */
    class ZDK_LOCAL OnSymbolTable
        : public unary_function<ImportPtr<DebuggerPlugin>, void>
    {
        typedef void (DebuggerPlugin::*Fun)(SymbolTable*);

    public:
        OnSymbolTable(SymbolTable& symtab, Fun fun)
            : symtab_(symtab), fun_(fun)
        { }

        void operator()(ImportPtr<DebuggerPlugin> plugin)
        {
            assert(!plugin.is_null());

            try
            {
                (plugin.get()->*fun_)(&symtab_);
            }
            catch (const exception& e)
            {
                cerr << "OnSymbolTable: " << e.what() << endl;
            }
        }

    private:
        SymbolTable& symtab_;
        const Fun fun_;
    };

} /********** Helpers **********/


SymbolEvents::SymbolEvents(DebuggerEngine& engine)
    : engine_(engine)
    , loadDynamicSyms_(true)
    , prevAddr_(0)
{
    if (const char* var = getenv("ZERO_NO_DYNAMIC_SYMBOLS"))
    {
        loadDynamicSyms_ = !strcmp(var, "0");
    }
    engine_.enum_plugins(this);
}


SymbolEvents::~SymbolEvents() throw()
{
}


void SymbolEvents::notify(DebuggerPlugin* pln)
{
    Lock<Mutex> lock(mutex_);
    if (DebugInfoReader* dinfo = interface_cast<DebugInfoReader*>(pln))
    {
        debugInfo_.push_back(dinfo);
    }
}


bool SymbolEvents::use_lazy_loading() const
{
    // if true, then load symbol tables as needed
    static bool lazy = env::get_bool("ZERO_LAZY_SYMBOLS", true);
    return lazy;
}


void SymbolEvents::on_init(SymbolTable& symtab)
{
    Lock<Mutex> lock(mutex_);
    SymbolTableEvents::on_init(symtab);

    OnSymbolTable pred(symtab, &DebuggerPlugin::on_table_init);
    engine().for_each_plugin<OnSymbolTable&>(pred);
}


/**
 * Done reading the table, report some numbers to console.
 * Also, restore breakpoints in the module that just loaded.
 */
void SymbolEvents::on_done(SymbolTable& table)
{
    Lock<Mutex> lock(mutex_);

#ifdef INDEX_TABLES_BY_SOURCE
    //
    // experimental
    //
    map_sources(table);

#endif
    if (table.is_loaded())
    {
        // "broadcast" the event to all plugins
        OnSymbolTable pred(table, &DebuggerPlugin::on_table_done);
        engine().for_each_plugin<OnSymbolTable&>(pred);
    }

    if (prevAddr_ != table.addr())
    {
        if (engine().breakpoint_manager())
        {
            prevAddr_ = table.addr();

            RefPtr<ModuleImpl> module(new ModuleImpl(table));
            ZObjectScope scope;
            if (Process* proc = table.process(&scope))
            {
                engine().restore_module(*proc, *module);
            }
        }
    }
}


/**
 * The symbols for which this function returns false
 * are rejected (i.e. not memorized in the table)
 */
bool SymbolEvents::on_symbol(SymbolTable& symtab, Symbol& sym)
{
    bool result = true;

    //Lock<Mutex> lock(mutex_);
    if (!SymbolTableEvents::on_symbol(symtab, sym))
    {
        return false;
    }
    if (symtab.is_dynamic() && !loadDynamicSyms_)
    {
        result = false;
    }
    return result;
}


/**
 * Translate address into line and source file name
 */
size_t
SymbolEvents::addr_to_line(const SymbolTable&      tbl,
                           addr_t                  addr,
                           RefPtr<SharedString>&   source)
{
    size_t line = 0;
    Lock<Mutex> lock(mutex_);

    AddrToLineHelper helper;

    DebugInfo::const_iterator i = debugInfo_.begin();
    for (addr_t nearest = 0; i != debugInfo_.end(); ++i)
    {
        (*i)->addr_to_line(&tbl, addr, &nearest, &helper);
    }
    if ((line = helper.line()) != 0)
    {
        source = helper.source();
        string path = source->c_str();
        ZObjectScope scope;
        if (map_path(tbl.process(&scope), path))
        {
            source = shared_string(path);
        }
    }
    return line;
}


SharedString*
SymbolEvents::reverse_map_path(const RefPtr<SharedString>& file) const
{
    PathMap::const_iterator j = pathMap_.find(file);
    if (j != pathMap_.end())
    {
        return j->second.get();
    }
    return file.get();
}


/**
 * Enumerate the addresses of code generated for
 * the given filename and source line.
 */
size_t
SymbolEvents::line_to_addr(const SymbolTable&      tbl,
                           RefPtr<SharedString>    file,
                           size_t                  line,
                           EnumCallback<addr_t>*   observ)
{
    Lock<Mutex> lock(mutex_);
    size_t count = 0;

    file = reverse_map_path(file);

    DebugInfo::const_iterator i = debugInfo_.begin();
    for (ZObjectScope scope; i != debugInfo_.end(); ++i)
    {
        count += (*i)->line_to_addr(tbl.process(&scope),
                                    tbl.filename(),
                                    tbl.adjustment(),
                                    file.get(),
                                    line, observ);
    }
    return count;
}


/**
 * Given a symbol, retrieve the address immediately
 * greater than sym->addr() for which line number info exists
 */
addr_t SymbolEvents::next_line(const RefPtr<Symbol>& sym) const
{
    addr_t addr = 0;
    Lock<Mutex> lock(mutex_);

    ZObjectScope scope;
    if (SymbolTable* table = sym->table(&scope))
    {
        DebugInfo::const_iterator i = debugInfo_.begin();
        for (; i != debugInfo_.end() && !addr; ++i)
        {
            addr = (*i)->next_line_addr(table,
                                        sym->value() + sym->offset(),
                                        sym->file(),
                                        sym->line());
            if (addr)
            {
                addr += table->adjustment();
            }
        }
    }
    return addr;
}


/**
 * For SymbolTable::enum_deferred_breakpoints.
 */
void SymbolEvents::notify(BreakPoint* bpnt, const SymbolTable* symTab)
{
    assert(bpnt);
    assert(symTab);

    Lock<Mutex> lock(mutex_);
    engine_.activate_deferred_breakpoint(bpnt, *symTab);
}


/**
 * Isolate SymbolTableImpl from ModuleImpl
 */
RefPtr<Module> SymbolEvents::get_module(SymbolTable& table) const
{
    return new ModuleImpl(table);
}


BreakPoint*
SymbolEvents::set_deferred_breakpoint(Runnable* runnable,
                                      BreakPoint::Type type,
                                      Symbol& symbol,
                                      BreakPoint::Action* action)
{
    RefPtr<BreakPoint::Action> bpntAct = action;
    if (!action)
    {
        bpntAct = engine_.interactive_action("USER");
        action = bpntAct.get();
    }
    assert(type != BreakPoint::ANY);

    BreakPoint* bpnt = NULL;
    if (BreakPointManager* mgr = engine_.breakpoint_manager())
    {
        bpnt = mgr->set_breakpoint(runnable,
                                   type,
                                   symbol.addr(),
                                   action,
                                   true, // deferred
                                   &symbol);
        assert(bpnt);
    }
    if (bpnt)
    {
        assert(bpnt->is_deferred());
        assert(bpnt->symbol());
        assert(bpnt->symbol()->addr() == symbol.addr());
    }
    return bpnt;
}


void SymbolEvents::map_sources(SymbolTable& table)
{
    ZObjectScope scope;
    currentTable_ = &table;
    for (DebugInfo::iterator i = debugInfo_.begin(); i != debugInfo_.end(); ++i)
    {
        (*i)->lookup_unit_by_name(table.process(&scope), table.filename(), 0, this);
    }
    currentTable_.reset();
}


bool SymbolEvents::notify(TranslationUnit* unit)
{
    assert(unit);
    unit->enum_sources(this);
    return true;
}


bool SymbolEvents::notify(SharedString* src)
{
    RefPtr<SharedString> path = shared_string(canonical_path(src->c_str()));
    sourceMap_.insert(make_pair(path, currentTable_));
    return true;
}


/*
size_t SymbolEvents::enum_tables_by_source
(
    SharedString* source,
    EnumCallback<SymbolTable*>* callback
) const
{
    source = reverse_map_path(source);

    pair<SourceMap::const_iterator,
         SourceMap::const_iterator> range = sourceMap_.equal_range(source);

    if (callback)
    {
        for (; range.first != range.second; ++range.first)
        {
            if (RefPtr<SymbolTable> symtab = range.second->second.ref_ptr())
            {
            #ifdef DEBUG
                clog << __func__ << ": " << symtab->filename() << endl;
            #endif
                callback->notify(symtab.get());
            }
        }
    }
    return distance(range.first, range.second);
}
*/


bool SymbolEvents::map_path(Process* proc, string& path) const
{
    const string origPath = path;
    bool result = engine_.map_path(proc, path);
    if (result)
    {
        if (StringCache* cache = interface_cast<TypeSystem*>(proc))
        {
            pathMap_.insert(
                make_pair(cache->get_string(path.c_str(), path.size()),
                          cache->get_string(origPath.c_str(), origPath.size())
                         ));
        }
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
