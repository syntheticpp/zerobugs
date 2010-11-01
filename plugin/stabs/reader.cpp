//
// $Id: reader.cpp 729 2010-10-31 07:00:15Z root $
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
#include <stdexcept>
#include "reader.h"
#include "dbgout.h"
#include "dharma/canonical_path.h"
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "zdk/argv_util.h"
#include "zdk/check_ptr.h"
#include "zdk/frame_handler.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "zdk/type_system_util.h"
#include "zdk/zero.h"
#include "zdk/zobject_scope.h"
#include "elfz/public/binary.h"
#include "stabz/public/compile_unit.h"
#include "stabz/public/function.h"
#include "stabz/public/fwdtype.h"
#include "stabz/public/variable.h"
#include "typez/public/debug_symbol_impl.h"
#include "unmangle/unmangle.h"

using namespace std;
using namespace Stab;


#define this_func string("Stab::Reader::") + __func__

/**
 * Threshold for calling the progress callback. No progress
 * indication is given when processing a block of STABS
 * of lesser entries
 */
static const size_t PROGRESS_THRESHOLD =
    env::get("ZERO_STABS_PROGRESS", 20000);


int32_t query_version(int32_t* minor)
{
    if (minor) *minor = ZDK_MINOR;
    return ZDK_MAJOR;
}


void query_plugin(InterfaceRegistry* registry)
{
    registry->update(DebuggerPlugin::_uuid());
    registry->update(DebugInfoReader::_uuid());
}


Plugin* create_plugin(uuidref_t iid)
{
    Plugin* plugin = 0;

    if (uuid_equal(DebuggerPlugin::_uuid(), iid)
     || uuid_equal(DebugInfoReader::_uuid(), iid))
    {
        plugin = new Stab::Reader();
    }
    return plugin;
}


Reader::Reader()
    : debugLevel_(0)
    , debugger_(0)
    , percent_(0.0)
    , numEntries_(0)
    , lazy_(env::get_bool("ZERO_STABS_LAZY", true))
    , showProgress_(true)
    , attached_(false)
{
    descriptors_.set_empty_key(RefPtr<SharedString>());
}


Reader::~Reader() throw()
{
}


void Reader::release()
{
    delete this;
}




bool Reader::initialize(Debugger* debugger, int* ac, char*** av)
{
    debugger_ = debugger;

    BEGIN_ARG_PARSE(ac, av)
        ON_ARG("--stabs-debug") { ++debugLevel_; }
        ON_ARG("--stabs-lazy")  { lazy_ = true; }
        ON_ARG("--stabs-now")   { lazy_ = false; }
        ON_ARG("--stabs-no-progress") { showProgress_ = false; }
    END_ARG_PARSE

    return true;
}


void Reader::shutdown()
{
    if (attached_)
    {
        on_detach(0);
    }
}


void Reader::register_streamable_objects(ObjectFactory*)
{
}


void Reader::on_table_init(SymbolTable*)
{
}


void Reader::on_table_done(SymbolTable*)
{
}

void Reader::on_attach(Thread* thread)
{
    dbgout(0) << "attached: " << thread->lwpid() << endl;
    attached_ = true;
}


void Reader::on_detach(Thread* thread)
{
    if (!attached_)
    {
        return;
    }
    if (thread == 0) // detached from all threads?
    {
        attached_ = false;
        descriptors_.clear();
        file_.reset();
        dbgout(0) << "all descriptors discarded.\n";
    }
}


bool Reader::on_event(Thread*, EventType)
{
    return false;
}


void Reader::on_program_resumed()
{
}


void Reader::on_insert_breakpoint(volatile BreakPoint*)
{
}


void Reader::on_remove_breakpoint(volatile BreakPoint*)
{
}


bool Reader::on_progress(const char*, double, word_t)
{
    return true;
}


/**************************************************************
 * Translate Stab::Variable objects into DebugSymbol objects
 */
class EmitVar : private DebugSymbolCallback
{
BEGIN_INTERFACE_MAP(EmitVar)
    // empty
END_INTERFACE_MAP()

public:
    EmitVar
    (
        DebugInfoReader*    reader,
        RefPtr<CompileUnit> unit,
        DebugSymbolEvents*  events,
        RefPtr<Thread>      thread,
        RefPtr<Symbol>      func,
        Frame*              frame = 0,
        const char*         name = 0
    )
      : reader_(reader)
      , unit_(unit)
      , events_(events)
      , thread_(thread)
      , func_(func)
      , frame_(frame ? frame->frame_pointer() : 0)
      , name_(name)
      , count_(0)
      , types_(*CHKPTR(interface_cast<TypeSystem*>(thread.get())))
      , emitParam_(false)
    {
        assert(!func_.is_null());
        assert(!unit_.is_null());
    }

    ////////////////////////////////////////////////////////////
    void operator()(const RefPtr<Stab::Block>& blk)
    {
        assert(!blk.is_null());
        const addr_t addr = func_->value() + func_->offset();

        // There are some cases when we don't know the ending address
        // of the block or function; in such cases, prefer to err
        // on the side of showing more variables rather than less.
        if ((addr >= blk->begin_addr()) &&
            ((addr <= blk->end_addr()) || (blk->end_addr() == 0)))
        {
            // emit variables in block (or function)
            blk->for_each_var<EmitVar&>(*this).count();

            // dig into sub-blocks
            blk->for_each_block<EmitVar&>(*this).count();
        }
 /* #if DEBUG
        else
        {
            clog << "addr is outside scope: " << hex << addr;
            clog << " [" << blk->begin_addr();
            clog << "-"  << blk->end_addr() << "]\n" << dec;
        }
    #endif */
    }

    ////////////////////////////////////////////////////////////
    // Translate Stab::Variable into DebugSymbol
    void operator()(const RefPtr<Stab::Variable>& var)
    {
        assert(!thread_.is_null());
        if (name_ && *name_ && !var->name().is_equal(name_))
        {
            return;
        }

        if (!emitParam_ && interface_cast<Stab::Parameter*>(var.get()))
        {
            return;
        }
        ZObjectScope scope;
        const SymbolTable* table = func_->table(&scope);
        if (!table)
        {
            return;
        }
        addr_t vaddr = var->addr(table->adjustment(), frame_);
        ++count_;

        RefPtr<DebugSymbolImpl> debugSym =
            DebugSymbolImpl::create(reader_,
                                    *thread_,
                                    *CHKPTR(var->type().ref_ptr()),
                                    var->name(),
                                    vaddr);
        assert(debugSym.get()); // expect bad_alloc otherwise

        if (events_ && events_->notify(debugSym.get()))
        {
            debugSym->read(events_);
        }
        ensure_type_info(debugSym.get());
    }

    size_t count() const { return count_; }

protected:
    /**
     * Make sure that the type information is parsed --
     * the "lazy" algorithm (that I am experimenting with)
     * does not go through the compilation units linearly.
     */
    void ensure_type_info(DataType* type)
    {
        if (thread_.is_null())
        {
            return;
        }

        // query for the type system interface
        TypeSystem* types = CHKPTR(interface_cast<TypeSystem*>(thread_.get()));

        ForwardType* fwd = interface_cast<ForwardType*>(type);

        if (fwd && !fwd->link() && types)
        {
            RefPtr<Descriptor> desc = unit_->descriptor();
            CHKPTR(desc); // should not be null at this point

        #if 1
            ELF::Binary elf(desc->name().c_str());

            Descriptor::const_iterator i = desc->begin();
            for (; i != desc->end(); ++i)
            {
                (*i)->parse(*CHKPTR(types), &elf);

                RefPtr<DataType> type = (*i)->get_type(types_, fwd->typeID(), true);
                if (!interface_cast<ForwardType>(type))
                {
                    fwd->link(type.get());
                    break;
                }
            }
        #else
            desc->parse(*CHKPTR(types));
            Descriptor::const_iterator i = desc->begin();
            for (; i != desc->end(); ++i)
            {
                DataType& type = (*i)->get_type(types_, fwd->typeID(), true);
                if (!interface_cast<ForwardType*>(&type))
                {
                    fwd->link(&type);
                    break;
                }
            }
        #endif
        }
    }

    /**
     * make sure we have the data type for sym
     */
    void ensure_type_info(DebugSymbol* sym)
    {
        if (sym)
        {
            assert(sym->type());
            ensure_type_info(sym->type());

            sym->enum_children(this);
        }
    }

    bool notify(DebugSymbol* sym)
    {
        ensure_type_info(sym);
        return true;
    }

protected:
    void set_frame_base(addr_t addr) { frame_ = addr; }

    void set_emit_params() { emitParam_  = true; }

private:
    EmitVar(const EmitVar&);
    EmitVar& operator=(const EmitVar&);

private:
    DebugInfoReader*    reader_;
    RefPtr<Descriptor>  desc_;
    RefPtr<CompileUnit> unit_;
    DebugSymbolEvents*  events_;
    RefPtr<Thread>      thread_;
    RefPtr<Symbol>      func_;
    addr_t              frame_;
    const char*         name_;
    size_t              count_;
    TypeSystem&         types_;
    bool                emitParam_;
};


class EmitParam : public EmitVar
{
public:
    EmitParam
    (
        DebugInfoReader*    reader,
        RefPtr<CompileUnit> unit,
        DebugSymbolEvents*  events,
        RefPtr<Thread>      thread,
        RefPtr<Symbol>      func,
        Frame*              frame = 0,
        const char*         name = 0
    )
      : EmitVar(reader, unit, events, thread, func, frame, name)
    {
        if (frame && func.get())
        {
            set_frame_base(thread_frame_base(*thread, *frame, *func));
        }
        set_emit_params();
    }
};


/**
 * Lookup the Stab::Descriptor, compilation unit within descriptor
 * and function within unit, and emit a DebugSymbol for each var.
 */
size_t Reader::enum_locals (
    Thread*             thread,
    const char*         name,
    Frame*              frame,
    Symbol*             funcSym,
    DebugSymbolEvents*  events,
    bool                paramOnly)
{
    assert(thread);
    assert(funcSym);

    if (!thread || !funcSym) // just to be defensive
    {
        return 0;
    }
    // relative-based addr:
    const addr_t addr = funcSym->value() + funcSym->offset();
    ZObjectScope scope;
    const SymbolTable* table = funcSym->table(&scope);
    if (!table)
    {
        return 0;
    }
    SharedString* filename = CHKPTR(table->filename());

    dbgout(0) << "enum_locals: frame base="
              << (void*)CHKPTR(frame)->frame_pointer()
              << ": " << funcSym->name()
              << " in " << filename << endl;

    RefPtr<Descriptor> desc(get_descriptor(filename, lazy_ ? NULL : thread));
    if (desc.is_null())
    {
        dbgout(0) << "get_descriptor()=NULL\n";
        return 0;
    }
    RefPtr<CompileUnit> unit = desc->get_compile_unit(addr);
    if (unit.is_null())
    {
        dbgout(0) << "unit not found\n";
        return 0;
    }
    dbgout(0) << unit->name().c_str() << endl;

    ensure_parsed(*thread, *unit);

    RefPtr<Function> func = unit->lookup_function(addr, false);
    if (func.is_null())
    {
        dbgout(0) << "function not found\n";
        return 0;
    }
    size_t count = 0; // return count of enumerated vars

    dbgout(0) << func->name().c_str() << endl;
    dbgout(0) << func->variables().size() << " local var(s)\n";
    dbgout(0) << func->blocks().size() << " lexical block(s)\n";

    if (!paramOnly) // okay to emit variables
    {
        EmitVar emitVar(this, unit, events, thread, funcSym, frame, name);
        emitVar(RefPtr<Block>(func));
        count += emitVar.count();
    }
    EmitParam emitParam(this, unit, events, thread, funcSym, frame, name);

    const Block::VarList& param = func->params();

    for_each<Block::VarList::const_iterator, EmitParam&>(
        param.begin(), param.end(), emitParam);

    count += emitParam.count();
    return count;
}


/**
 * Globals may have a zero address in the STAB entry, in
 * which case we need to get the address from the ELF symbols.
 */
static addr_t resolve_global_addr(Thread& thread, const char* name)
{
    addr_t addr = 0;
    SymbolEnum symbols;

    if (thread.symbols()->enum_symbols(name, &symbols))
    {
        // keep the first match only; can there be any
        // ambiguities here?
        addr += symbols.front()->addr();
    }
    return addr;
}


/**
 * Lookup global symbols of given name, and notify
 * the events for all matches (if EVENTS is not null).
 */
size_t Reader::enum_globals (
    Thread*             thread,
    const char*         name,
    Symbol*             sym,
    DebugSymbolEvents*  events,
    LookupScope         scope,
    bool                enumFuncs)
{
    if (!thread)
    {
        throw invalid_argument("null thread in " + this_func);
    }
    if (!sym)
    {
        throw invalid_argument("null symbol in " + this_func);
    }
    size_t result = 0;
    ZObjectScope objectScope;

    if (scope == LOOKUP_ALL)
    {
        RefPtr<SymbolMap::LinkData> link =
            CHKPTR(thread->symbols())->file_list();

        // look in all modules
        for (; !link.is_null(); link = link->next())
        {
            SharedString* linkpath = CHKPTR(link->filename());

            RefPtr<Descriptor> desc = get_descriptor(linkpath, thread);
            if (desc.is_null())
            {
                continue;
            }
            dbgout(0) << linkpath << endl;
            result += enum_globals(*thread, name, *sym, events, *desc, enumFuncs);
        }
    }
    else if (const SymbolTable* table = sym->table(&objectScope))
    {
        SharedString* filename = table->filename();
        RefPtr<Descriptor> desc = get_descriptor(CHKPTR(filename), thread);
        if (desc.is_null())
        {
            dbgout(0) << "null descriptor\n";
        }
        else if (scope == LOOKUP_MODULE)
        {
            // enum globals in all compilation units of this module
            result += enum_globals(*thread, name, *sym, events, *desc, enumFuncs);
        }
        else if (scope == LOOKUP_UNIT)
        {
            const addr_t addr = sym->value() + sym->offset();

            RefPtr<CompileUnit> unit = desc->get_compile_unit(addr);
            if (unit.is_null())
            {
                dbgout(0) << "NULL unit\n";
            }
            else
            {
                result = enum_globals(*thread, name, *sym, events, *unit, enumFuncs);
            }
        }
        else
        {
            clog << "*** Warning: unhandled scope " << scope << endl;
        }
    }
    return result;
}


size_t Reader::enum_globals(
    Thread&             thread,
    const char*         name,
    Symbol&             sym,
    DebugSymbolEvents*  events,
    Descriptor&         desc,
    bool                enumFuncs)
{
    ensure_parsed(thread, desc);

    size_t result = 0;
    Descriptor::iterator u = desc.begin();
    const Descriptor::iterator end = desc.end();

    for (; u != end; ++u)
    {
        result += enum_globals(thread, name, sym, events, **u, enumFuncs);
    }
    return result;
}


size_t Reader::enum_globals(
    Thread&             thread,
    const char*         name,
    Symbol&             funcSym,
    DebugSymbolEvents*  events,
    Stab::CompileUnit&  unit,
    bool                enumFuncs)
{
    ensure_parsed(thread, unit);

    typedef CompileUnit::GlobalVarMap::const_iterator iterator;
    pair<iterator, iterator> range;

    if (name)
    {
        RefPtr<SharedString> key(shared_string(name));
        range = unit.globals().equal_range(key);
    }
    else
    {
        range.first = unit.globals().begin();
        range.second = unit.globals().end();
    }

    size_t result = 0;
    if (events)
    {
        EmitVar emit(this, &unit, events, &thread, &funcSym);

        for (; range.first != range.second; ++range.first)
        {
            RefPtr<Variable> var = range.first->second;
            if (var->offset() == 0)
            {
                var->set_offset(resolve_global_addr(thread, name));
            }
            emit(var);
        }
        result = emit.count();
    }
    else
    {
        result = distance(range.first, range.second);
    }
    if (!result && enumFuncs && name)
    {
        result += enum_functions(unit, thread, name, events);
    }
    return result;
}


/**
 * This function is invoked by enum_globals.
 * Purpose: support function identifiers in expressions
 * @todo: include class methods if `this' is visible in scope
 */
size_t Reader::enum_functions(
    CompileUnit&        unit,
    Thread&             thread,
    const char*         name,
    DebugSymbolEvents*  events)
{
    assert(thread.symbols());
    assert(thread.debugger());

    FunctionEnum funcs;
    thread.symbols()->enum_symbols(name, &funcs);

    size_t result = 0;

    TypeSystem* types = &interface_cast<TypeSystem&>(thread);

    FunctionEnum::const_iterator i = funcs.begin();
    for (; i != funcs.end(); ++i)
    {
        RefPtr<Function> f = unit.lookup_function((*i)->addr(), true);
        if (f.is_null())
        {
            continue;
        }

        assert(f->return_type().get());

        bool varArgsOk = false;
        size_t len = f->name().length();
        if (char* fn = unmangle(f->name().c_str(), &len, 0, UNMANGLE_NOFUNARGS))
        {
            varArgsOk = strstr(fn, "...");
            free(fn);
        }
        else // looks like an extern "C" function
        {
            varArgsOk = true;
        }

        RefPtr<DataType> funType =
            get_function_type(  *types,
                                f->return_type(),
                                f->param_types(),
                                varArgsOk);

        assert(funType.get()); // by contract

        ++result;

        if (events)
        {
            RefPtr<DebugSymbol> funSym =
                DebugSymbolImpl::create(this,
                                        thread,
                                        *funType,
                                        *(*i)->demangled_name(false),
                                        (*i)->addr());
            events->notify(funSym.get());
        }
    }
    return result;
}


size_t Reader::addr_to_line (
    const SymbolTable*                      symTable,
    addr_t                                  addr,
    addr_t*                                 nearest,
    EnumCallback2<SharedString*, size_t>*   events)
{
    assert(symTable);
    RefPtr<Descriptor> desc = get_descriptor(symTable->filename(), (TypeSystem*)0);

    if (desc.is_null())
    {
        dbgout(0) << "NULL descriptor\n";
        return 0;
    }
    dbgout(0) << symTable->filename() << endl;

    RefPtr<Stab::CompileUnit> unit = desc->get_compile_unit(addr);
    if (unit.is_null())
    {
        dbgout(0) << "NULL unit 0x"
                  << hex << addr + symTable->adjustment() << dec << endl;
        return 0;
    }

    size_t result = unit->addr_to_line(addr, nearest, events);
    return result;
}


addr_t Reader::next_line_addr (
    const SymbolTable* symtab,
    addr_t addr,
    SharedString* srcFile,
    size_t srcLine
    ) const
{
    addr_t result = 0;
    DescriptorMap::const_iterator i = descriptors_.begin();

    for (; (result == 0) && (i != descriptors_.end()); ++i)
    {
        result = i->second->next_line(srcFile, srcLine, addr);
    }
    return result;
}



size_t Reader::line_to_addr (
    Process*                process,
    SharedString*           moduleName,
    loff_t                  moduleAdjust,
    SharedString*           file,
    size_t                  line,
    EnumCallback<addr_t>*   events,
    bool*                   cancelled)
{
    vector<addr_t> addrs;

/* handle the case where a source file is shared by several modules
    DescriptorMap::const_iterator i = descriptors_.begin();
    for (; i != descriptors_.end(); ++i)
    {
        vector<addr_t> tmp = i->second->line_to_addr(file, line);
        addrs.insert(addrs.end(), tmp.begin(), tmp.end());
    }
*/
    RefPtr<Descriptor> desc = get_descriptor(moduleName, (TypeSystem*)0);

    dbgout(0) << "desc=" << desc.get() << endl;
    if (!desc.is_null())
    {
        addrs = desc->line_to_addr(file, line);
    }
    if (events)
    {
        vector<addr_t>::const_iterator i = addrs.begin();
        for (; i != addrs.end(); ++i)
        {
            addr_t addr = *i;
            if (addr)
            {
                addr += moduleAdjust;
            }
            events->notify(addr);
        }
    }
    return addrs.size();
}



bool Reader::get_return_addr(Thread*, addr_t, addr_t*) const
{
    return false;
}



bool Reader::get_fun_range (
    Thread*     thread,
    addr_t      addr,
    addr_t*     low,
    addr_t*     high
    ) const
{
    CHKPTR(thread);
    CHKPTR(thread->symbols());

    SymbolTable* symtab = thread->symbols()->lookup_table(addr);
    if (symtab)
    {
        RefPtr<Descriptor> desc = get_descriptor(symtab->filename());
        if (desc.get())
        {
            RefPtr<CompileUnit> unit = desc->get_compile_unit(addr);
            if (unit.get())
            {
                assert_gte(addr, symtab->adjustment());
                addr -= symtab->adjustment();

                RefPtr<Function> func = unit->lookup_function(addr, false);
                if (func.get())
                {
                    if (low)
                    {
                        *low = func->begin_addr() + symtab->adjustment();
                    }
                    if (high)
                    {
                        *high = func->end_addr() + symtab->adjustment();
                    }
                    return true;
                }
            }
        }
    }
    return false;
}



DebugSymbol* Reader::get_return_symbol (
    Thread*         thread,
    const Symbol*   symbol, /* symbol corresponding to function */
    RefTracker*     tracker /* track ref counts to ensure that returned object is not leaked */
    )
{
    // pre-conditions
    assert(thread);
    assert(symbol);
    if (!thread || !symbol)
    {
        return NULL;
    }
    ZObjectScope scope;
    const SymbolTable* table = symbol->table(&scope);
    if (!table)
    {
        return NULL;
    }

    RefPtr<SharedString> file = table->filename();

    RefPtr<Descriptor> desc = get_descriptor(file.get(), thread);
    if (desc.is_null())
    {
        dbgout(0) << "descriptor not found.\n";

        // This is not a critical error, maybe the debug information
        // for the binary is not in STABS format
        return NULL;
    }

    // get the compilation unit where the function is defined
    RefPtr<CompileUnit> unit = desc->get_compile_unit(symbol->addr());
    if (unit.is_null())
    {
        dbgout(0) << "unit not found\n";
        // This is not a critical error, maybe the debug information
        // for the compilation unit is not in STABS format
        return NULL;
    }
    dbgout(0) << unit->name().c_str() << endl;

    ensure_parsed(*thread, *unit);

    assert_gte(symbol->addr(), table->adjustment());
    addr_t addr = symbol->addr() - table->adjustment();

    RefPtr<Function> func = unit->lookup_function(addr, false);
    if (func.is_null())
    {
        return NULL;
    }

    RefPtr<DataType> retType = func->return_type();
    if (retType.is_null())
    {
        string error("NULL return type: ");
        error += CHKPTR(symbol->demangled_name())->c_str();

        throw runtime_error(error);
    }

    addr = thread->result();

    RefPtr<DebugSymbolImpl> debugSymbol =
        DebugSymbolImpl::create(this,
                                *thread,
                                *retType,
                                *symbol->demangled_name(false),
                                addr,
                                NULL,
                                0,
                                true);
    if (tracker)
    {
        tracker->register_object(debugSymbol.get());
    }
    return debugSymbol.detach();
}


/**
 * @return the symbol table of the module (shared object, main executable)
 * that contains the program counter in the current scope.
 */
static RefPtr<SymbolTable> current_module_table(Thread* thread)
{
    Frame* frame = thread_current_frame(thread);
    if (!frame || !frame->function())
    {
        return NULL;
    }
    ZObjectScope scope;
    return frame->function()->table(&scope);
}


/**
 * Lookup type by name.
 * @note currently only LOOKUP_MODULE and LOOKUP_UNIT are supported
 * As a hack, LOOKUP_ALL fails over to LOOKUP_MODULE
 */
DataType* Reader::lookup_type (
    Thread*         thread,
    const char*     typeName,
    addr_t          addr,
    LookupScope     scope)
{
    assert(scope != LOOKUP_PARAM);
    //assert(thread);
    //assert(typeName);

    if (!thread || !typeName)
    {
        return NULL;
    }

    RefPtr<SymbolTable> table = current_module_table(thread);
    if (!table)
    {
        return NULL;
    }
    RefPtr<SharedString> name = shared_string(typeName);
    if (scope == LOOKUP_ALL)
    {
        scope = LOOKUP_MODULE; // fixme
    }

    RefPtr<DataType> type;

    if (RefPtr<Descriptor> desc = get_descriptor(CHKPTR(table->filename())))
    {
        if (scope == LOOKUP_UNIT)
        {
            assert_gte(addr, table->adjustment());
            addr -= table->adjustment();

            if (CompileUnit* unit = desc->get_compile_unit(addr))
            {
                ensure_parsed(*thread, *unit);
                type = unit->lookup_type(*name);
            }
        }
        else if (scope == LOOKUP_MODULE)
        {
            Descriptor::TypeMap& tmap = desc->type_map();
            Descriptor::TypeMap::const_iterator i = tmap.find(name);
            if (i != tmap.end())
            {
                type = i->second.ref_ptr();
            }
        }
    }
    if (ForwardType* fwd = interface_cast<ForwardType*>(type.get()))
    {
        type = fwd->link();
    }
    return type.get();
}


RefPtr<Descriptor> Reader::get_descriptor(SharedString* file) const
{
    if (!file)
    {
        return NULL;
    }
    RefPtr<Descriptor> result;
    DescriptorMap::const_iterator i = descriptors_.find(file);
    if (i != descriptors_.end())
    {
        result = i->second;
    }
    else
    {
        dbgout(0) << "not found " << file << endl;
    }
    return result;
}



RefPtr<Descriptor> Reader::get_descriptor(SharedString* file, Thread* thread)
{
    TypeSystem* types = NULL;
    if (!lazy_)
    {
        types = interface_cast<TypeSystem*>(thread);
        if (!types)
        {
            assert(!thread);
        }
    }
    return get_descriptor(file, types);
}


RefPtr<Descriptor> Reader::get_descriptor(SharedString* file, TypeSystem* types)
{
    if (!file)
    {
        throw runtime_error("get_descriptor: NULL file");
    }
    RefPtr<Descriptor> result;

    if (attached_)
    {
        dbgout(0) << file << endl;

        DescriptorMap::iterator i = descriptors_.find(file);
        if (i == descriptors_.end())
        {
            result = new Descriptor(*file);

            if (lazy_ || (types == NULL))
            {
                result->init();
            }
            else
            {
                result->set_observer_events(this);
                result->init_and_parse(*CHKPTR(types));
            }
            descriptors_.insert(i, make_pair(file, result));
        }
        else
        {
            result = i->second;
            // ensure that all .STAB entries are parsed
            if (types)
            {
                result->set_observer_events(this);
                result->parse(*types);
            }
        }
    }
    return result;
}


TranslationUnit* Reader::lookup_unit_by_addr(Process*, SharedString* fname, addr_t addr)
{
    RefPtr<Stab::Descriptor> desc;
    if (fname)
    {
        // TODO: need to pass in the thread here?
        desc = get_descriptor(fname, (TypeSystem*)0);
    }
    if (desc)
    {
        return desc->get_compile_unit(addr);
    }
    return NULL;
}


static bool unit_matches(RefPtr<CompileUnit> unit, const char* filename)
{
    assert(filename);

    string path = canonical_path(unit->filename());

    if (strcmp(path.c_str(), filename) == 0)
    {
        return true;
    }
    const CompileUnit::SourceSet& sources = unit->sources();

    for (CompileUnit::SourceSet::const_iterator i = sources.begin();
            i != sources.end();
            ++i)
    {
        path = (*i)->c_str();

        if ((path == filename)
         || canonical_path(path.c_str()) == filename)
        {
            return true;
        }
    }
    return false;
}



size_t Reader::lookup_unit_by_name (
    Process*,
    SharedString* moduleFileName,
    const char* filename,
    EnumCallback<TranslationUnit*, bool>* callback)
{
    size_t count = 0;

    if (moduleFileName)
    {
        if (RefPtr<Stab::Descriptor> desc =
            get_descriptor(moduleFileName, (TypeSystem*)0))
        {
            Stab::Descriptor::const_iterator i = desc->begin();

            for (; i != desc->end(); ++i)
            {
                if (!filename || unit_matches(*i, filename))
                {
                    ++count;

                    if (callback)
                    {
                        if (!callback->notify(i->get()))
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    return count;
}


void Reader::on_section(const char* sec)
{
    if (sec)
    {
        section_ = sec;
    }
}


void Reader::on_begin(SharedString& file, const char* sec, size_t nEntries)
{
    if (sec)
    {
        section_ = sec;
    }

    reset_progress_info();
    file_ = &file;
    /* memorize number of entries, so that percentage can
       be calculated later, as progress is reported  */
    numEntries_ = nEntries;

    ostringstream msg;
    msg << "Reading: ";
    msg << file.c_str() << " (" << nEntries << " STABS entries)";
    debugger_->message(msg.str().c_str(), Debugger::MSG_STATUS);
}


bool Reader::on_stab(size_t index, const stab_t&, const char*, size_t)
{
    assert(debugger_);

    if (progress_.empty())
    {
        assert(!file_.is_null());
        ostringstream msg;

        if (file_->length() > 64)
        {
            msg << "...~" << file_->c_str() + file_->length() - 64;
        }
        else
        {
            msg << file_->c_str();
        }

        msg << ": section " << section_;
        msg << " (" << numEntries_ << " entries)";

        progress_ = msg.str();
    }

    if (showProgress_ && (numEntries_ > PROGRESS_THRESHOLD))
    {
        double perc = (index) / (double) numEntries_;
        if (perc > percent_ + 0.01)
        {
            percent_ = perc;
            return debugger_->progress(progress_.c_str(), perc);
        }
    }
    return true;
}


void Reader::on_done(size_t count)
{
    if (showProgress_
        && (numEntries_ > PROGRESS_THRESHOLD)
        && (count == numEntries_))
    {
        assert(debugger_);
        debugger_->progress(progress_.c_str(), 1.0);
    }
    reset_progress_info();
}


void Reader::reset_progress_info()
{
    progress_.clear();
    percent_  = .0;
    numEntries_ = 0;
}


void Reader::ensure_parsed(Thread& thread, Descriptor& desc) const
{
    if (lazy_)
    {
        // ensure that all .stab entries that correspond to the
        // translation unit are parsed (since the module descriptor
        // is being fetched lazily).

        desc.parse(interface_cast<TypeSystem&>(thread));
    }
}


void Reader::ensure_parsed(Thread& thread, Descriptor& desc)
{
    if (lazy_)
    {
        desc.set_observer_events(this);
        desc.parse(interface_cast<TypeSystem&>(thread));
    }
}


void Reader::ensure_parsed(Thread& thread, CompileUnit& unit)
{
    if (lazy_)
    {
        if (Descriptor* desc = unit.descriptor())
        {
            desc->set_observer_events(this);
        }
        unit.parse(interface_cast<TypeSystem&>(thread));
    }
}


void Reader::ensure_parsed(Thread& thread, CompileUnit& unit) const
{
    if (lazy_)
    {
        unit.parse(interface_cast<TypeSystem&>(thread));
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
