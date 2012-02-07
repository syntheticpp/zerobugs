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
#include "zdk/argv_util.h"
#include "zdk/assert.h"
#include "zdk/check_ptr.h"
#include "zdk/export.h"
#include "zdk/shared_string_impl.h"
#include "zdk/stack.h"
#include "zdk/stdexcept.h"
#include "zdk/thread_util.h"
#include "zdk/types.h"
#include "zdk/type_system.h"
#include "zdk/zobject_scope.h"
#include <assert.h>
#include <algorithm>
#include <sstream>
#include "generic/lock.h"
#include "generic/temporary.h"
#include "typez/public/debug_symbol.h"
#include "typez/public/types.h"
#include "typez/public/util.h"
#include "dharma/canonical_path.h"
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "dharma/syscall_wrap.h"
#include "dwarfz/public/abi.h"
#include "dwarfz/public/addr_ops.h"
#include "dwarfz/public/canonical_path.h"
#include "dwarfz/public/class_type.h"
#include "dwarfz/public/compile_unit.h"
#include "dwarfz/public/debug.h"
#include "dwarfz/public/function.h"
#include "dwarfz/public/global.h"
#include "dwarfz/public/impl.h"
#include "dwarfz/public/init.h"
#include "dwarfz/public/inlined_instance.h"
#include "dwarfz/public/location.h"
#include "dwarfz/public/namespace.h"
#include "dwarfz/public/parameter.h"
#include "dwarfz/public/variable.h"
#include "dwarfz/public/type.h"
#include "dwarfz/public/unwind.h"
#include "dwarfz/private/factory.h"
#include "unmangle/unmangle.h"
#include "addr_operations.h"
#include "const_value.h"
#include "dbgout.h"
#include "frame.h"
#include "type_map.h"
#include "type_adapter.h"
#include "unit.h"
#include "reader.h"


using namespace std;
using namespace boost;
using namespace Dwarf;

typedef std::shared_ptr<InlinedInstance> InlinePtr;


static const char copyright_text[] =
  "Based on libdwarf,\n"
  "Copyright (C) 2000,2001,2003,2004,2005,2006 Silicon Graphics, Inc.\n"
  "All Rights Reserved. Portions Copyright 2002,2007 Sun Microsystems, Inc.\n"
  "All rights reserved. Portions Copyright 2007 David Anderson. All rights reserved.\n"
  "libdwarf is available at: http://reality.sgiweb.org/davea/dwarf.html\n"
  "The complete copyright terms for libdwarf are described in the\n"
  "LIBDWARFCOPYRIGHT file, shipped with the ZeroBUGS software.";


static bool inline_heuristics()
{
    static const bool f = env::get_bool("ZERO_INLINE_HEURISTICS", false);
    return f;
}



/**
 * Plugin entry points
 */
int32_t query_version(int32_t* minor)
{
    if (minor) *minor = ZDK_MINOR;
    return ZDK_MAJOR;
}


void query_plugin(InterfaceRegistry* registry)
{
    if (registry->update(DebuggerPlugin::_uuid()))
    {
        registry->update(FrameHandler::_uuid());
    }
}


Plugin* create_plugin(uuidref_t iid)
{
    static Plugin* plugin = 0;

    if (uuid_equal(DebuggerPlugin::_uuid(), iid)
     || uuid_equal(FrameHandler::_uuid(), iid))
    {
        if (plugin == 0)
        {
            plugin = new Reader();
        }
        return plugin;
    }
    return 0;
}



namespace
{
    /**
     * Adapts a EnumCallback2<SharedString*, size_t> interface
     * into a Dwarf::SourceLineEvents interface.
     * @see Reader::sourceLine()
     * @note:this is a by-product of libdwarfz being designed
     * as a standalone C++ wraper for libdwarf
     */
    class ZDK_LOCAL LineCallbackAdapter : public Dwarf::SrcLineEvents
    {
        typedef EnumCallback2<SharedString*, size_t> LineEvents;
        typedef EnumCallback<addr_t> AddrEvents;

        loff_t      adjust_;
        size_t      count_;
        LineEvents* lineEvents_;
        AddrEvents* addrEvents_;

    public:
        virtual ~LineCallbackAdapter() throw() {}

        LineCallbackAdapter
        (
            loff_t      adjust,
            LineEvents* lineEvents,
            AddrEvents* addrEvents
        )
          : adjust_(adjust)
          , count_(0)
          , lineEvents_(lineEvents)
          , addrEvents_(addrEvents)
        { }

        size_t count() const { return count_; }

        void on_done() { }

        bool on_srcline(SharedString* src,
                        Dwarf_Unsigned line,
                        Dwarf_Addr addr)
        {
            if (src && lineEvents_)
            {
                dbgout(1) << src->c_str() << ':' << line << endl;

                assert(src->c_str()[0] == '/'); // expect abs path
                lineEvents_->notify(src, line);
            }
            if (addrEvents_)
            {
                addrEvents_->notify(adjust_ + addr);
            }
            ++count_;
            return true;
        }
    };


    /**
     * Emit macros as if they where constant strings
     */
    class ZDK_LOCAL MacroEmitter : public MacroEvents
    {
        static size_t const maxCount_;

    public:
        MacroEmitter
        (
            const CompileUnit& unit,
            Thread& thread,
            DebugSymbolEvents& events,
            const char* name,
            Symbol* sym
        )
            : unit_(unit)
            , thread_(thread)
            , events_(events)
            , name_(name)
            , sym_(sym)
        { }

        virtual ~MacroEmitter() { }

        size_t enumerate()
        {
            unit_.enum_macros(this, maxCount_);

            if (!value_.empty())
            {
                emit(name_, value_);
                return 1;
            }
            return 0;
        }

    private:
        bool on_macro(const Dwarf_Macro_Details&);

        void emit(const string&, const string&);

    private:
        typedef map<RefPtr<SharedString>, size_t> IncludeMap;

        const CompileUnit&  unit_;
        Thread&             thread_;
        DebugSymbolEvents&  events_;
        string              name_;
        string              value_;
        RefPtr<Symbol>      sym_; // current scope
        IncludeMap          includes_;
    }; // MacroEmitter
}

/// @note 0 means unlimited
const size_t MacroEmitter::maxCount_ = env::get("ZERO_MAX_MACRO", 5000);


bool MacroEmitter::on_macro(const Dwarf_Macro_Details& detail)
{
    assert(detail.dmd_fileindex);
    Dwarf_Signed n = detail.dmd_fileindex - 1;

    RefPtr<SharedString> file = unit_.filename_by_index(n);

    switch (detail.dmd_type)
    {
    case DW_MACINFO_undef:
        if (name_ == detail.dmd_macro)
        {
            file = unit_.filename_by_index(n);

            // if we are in a line that is past the #undef,
            // or if we are in a different translation unit,
            // then undefine the macro
            if ((size_t)detail.dmd_lineno < sym_->line()
                || (file && file->length() && !sym_->file()->is_equal2(file.get()))
               )
            {
                value_.clear();
            }
        }
        break;

    case DW_MACINFO_define:
      {
        if (strncmp(detail.dmd_macro, name_.c_str(), name_.size()))
        {
            break;
        }
        const char c = detail.dmd_macro[name_.size()];
        if (c != '(' && !isspace(c))
        {
            break;
        }
        if (!sym_->file()->is_equal2(file.get()))
        {
            IncludeMap::const_iterator iter = includes_.find(file);

            if (iter != includes_.end() &&
                iter->second > sym_->line())
            {
                break;
            }
        }
        char* p = dwarf_find_macro_value_start(detail.dmd_macro);
        if (!p)
        {
            break;
        }
        value_ = p;
        while (p > detail.dmd_macro && isspace(*(p - 1)))
        {
            --p;
        }
        if (value_.empty())
        {
            value_ = "1";
        }
        name_.assign(detail.dmd_macro, p);
        break;
      }
    case DW_MACINFO_start_file:
        includes_[file] = detail.dmd_lineno;
        break;
    } // end switch

    return true;
}


void MacroEmitter::emit(const string& name, const string& value)
{
    dbgout(1) << "name=" << name << endl;
    emit_macro(thread_, events_, name, value);
}


/**
 * Given an address inside of a function, determine whether
 * the surrounding code is the instance of (another) inlined function
 */
bool Reader::find_inlined_instance(const Block& block, addr_t addr) const
{
    if (addr != inlinedAddr_)
    {
        assert(inlinedBlocks_.empty());
        inlinedAddr_ = addr;
    }
    else if (inlinedBlocks_.empty())
    {
        inlinedAddr_ = 0;
        return false;
    }
    bool found = false;
    const InlinedBlocks& blocks = block.inlined_blocks();

    InlinedBlocks::const_iterator i = blocks.begin();
    const InlinedBlocks::const_iterator end = blocks.end();
    for (; i != end && !found; ++i)
    {
        InlinePtr x = *i;
        if (addr >= x->low_pc() && addr < x->high_pc())
        {
            inlinedBlocks_.push_back(x);
            find_inlined_instance(*x, addr);
            found = true;
        }
    }
    if (!found) // dive into sub-blocks
    {
        List<Block> subBlocks = block.blocks();
        List<Block>::const_iterator j = subBlocks.begin();
        for (; j != subBlocks.end() && !found; ++j)
        {
            if (find_inlined_instance(*j, addr))
            {
                found = true;
            }
        }
    }
    return found;
}


Reader::Reader()
    : debugger_(0)
    , shallow_(true)
    , attached_(false)
    , fhandler_(true)
    , inlinedAddr_(0)
{
}


Reader::~Reader() throw()
{
}


void Reader::release()
{
    delete this;
}


const char* Reader::copyright() const
{
    return copyright_text;
}


bool Reader::initialize(Debugger* db, int* argc, char*** argv)
{
    debugger_ = db;

    Dwarf::init();

BEGIN_ARG_PARSE(argc, argv)
    ON_ARG("--dwarf-nofh")
    {
        fhandler_ = false; // disable frame handler
    }
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


void Reader::on_table_done(SymbolTable* table)
{
}


void Reader::on_attach(Thread* thread)
{
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
        handleCache_.reset();

        typeMap_.clear();

        inlinedAddr_ = 0;
        inlinedBlocks_.clear();
        pc2InlinedOff_.clear();

        linkageMap_.clear();
    }
}


bool Reader::on_event(Thread* thread, EventType)
{
    return false;
}


void Reader::ensure_debug_cache_inited() const
{
    if (!handleCache_.get())
    {
        // The Dwarf::Debug handles keep the file open;
        // to make sure that we do not exceed the max
        // number of allowed open files, we limit the size
        // of the cache.
        const long size =
            env::get("ZERO_DWARF_CACHE_SIZE",
            int(sysconf(_SC_OPEN_MAX) / 2));

        handleCache_.reset(new DebugCache(size, debugger_));
    }
}

Handle Reader::get_debug_handle(
    SharedString* fname,
    const RefPtr<Process>& process) const
{
    dbgout(2) << "attached=" << attached_ << endl;
    Handle handle;

    Lock<Mutex> lock(mutex_);

    if (attached_ && fname)
    {
        ensure_debug_cache_inited();
        handle = handleCache_->get_handle(fname, process.get(), this);
    }
    return handle;
}


Handle Reader::get_debug_handle(
    ino_t inode,
    const RefPtr<Process>& process) const
{
    Handle handle;
    Lock<Mutex> lock(mutex_);
    if (attached_)
    {
        ensure_debug_cache_inited();
        handle = handleCache_->get_handle(inode, process.get());
    }
    return handle;
}


void Reader::query_unit_headers(const string& filename, vector<Dwarf_Unsigned>& units) const
{
}


namespace Dwarf
{
    /**
     * Given a function and a Die that corresponds to an object
     * that lives within that function, determine the name of the
     * source file.
     */
    RefPtr<SharedString> decl_file(const Function* fun, const Die& die)
    {
        RefPtr<SharedString> filename;

        if (fun)
        {
            if (const CompileUnit* unit = fun->unit())
            {
                const int nFile = die.decl_file();
                if (nFile > 0)
                {
                    filename = unit->filename_by_index(nFile - 1);
                }
            }
        }
        return filename;
    }

    /**
     * This functor translates objects of the Dwarf::Datum type
     * into DebugSymbol instances.
     */
    class ZDK_LOCAL EmitDebugSymbol : public unary_function<Dwarf::Datum, void>
    {
        EmitDebugSymbol(const EmitDebugSymbol&);
        EmitDebugSymbol& operator=(const EmitDebugSymbol&);

    public:
        EmitDebugSymbol
        (
            Reader&             reader,
            Thread*             thread,
            const char*         name,
            Frame*              frame,
            const Symbol*       sym,
            DebugSymbolEvents*  events,
            const Function*     fun = NULL // function of scope
        )
          : reader_(reader)
          , typeMap_(reader.type_map())
          , thread_(thread)
          , name_(name)
          , frame_(frame)
          , events_(events)
          , count_(0)
          , moduleAdjust_(0)
          , frameBase_(0)
          , unitBase_(0)
          , fun_(fun)
          , sym_(sym)
        {
            assert(frame_);
            if (sym)
            {
                ZObjectScope scope;
                if (SymbolTable* table = sym->table(&scope))
                {
                    // the base address is needed for evaluating global
                    // variables; automatic variables are relative to
                    // the frame base

                    moduleAdjust_ = table->adjustment();

                    if (moduleAdjust_ < 0)
                    {
                        clog << "*** Warning: negative module address: ";
                        clog << table->filename() << hex << moduleAdjust_;
                        clog << dec << endl;
                    }
                }
            }
            if (fun_)
            {
                if (const CompileUnit* unit = fun_->unit())
                {
                    unitBase_ = unit->base_pc();
                }
            }
        }

        ~EmitDebugSymbol() { }

        void operator()(const std::shared_ptr<Datum>& dat)
        {
            if (dat)
            {
                (*this)(*dat);
            }
        }

        void operator()(const Datum&);

        void operator()(const Function&);

        /**
         * @return number of symbols emitted
         */
        size_t count() const { return count_; }

        const Symbol* symbol() const { return sym_; }

        addr_t unit_base() const { return unitBase_; }

        /**
         * Compute the frame base of the current scope.
         * @note currently, DW_AT_frame_base is a function attribute,
         * but a more generic implementation like this one does not
         * hurt: the frame_base is supported and computed at a
         * lexical block level.
         */
        addr_t compute_frame_base() const
        {
            addr_t base = 0;
            if (frame_)
            {
                base = frame_->frame_pointer();
                if (frame_->program_count() != frame_->real_program_count())
                {
                    //we're dealing with an inlined function, bye
                    dbgout(1) << "inlined function? " << hex << base << dec << endl;
                    return base;
                }
            }

            if (fun_)
            {
                addr_t addr = frame_ ? frame_->program_count() : 0;
                addr = fun_->frame_base(moduleAdjust_, base, addr);
                if (addr)
                {
                    base = addr;
                }
            }
            return base;
        }

        addr_t frame_base() const
        {
            if (!frameBase_)
            {
                frameBase_ = compute_frame_base();
            }
            return frameBase_;
        }

    protected:
        void read_from_register(DataType&, addr_t, SharedString&);

        void emit_symbol (
            const Dwarf::Datum&,
            std::shared_ptr<Dwarf::Type>,
            addr_t, /* address or value */
            addr_t, /* frame base       */
            bool    /* isRegister       */,
            bool    /* isValue          */);

    private:
        Reader&             reader_;
        TypeMap&            typeMap_;
        RefPtr<Thread>      thread_;
        const char* const   name_;
        Frame*              frame_;
        DebugSymbolEvents*  events_;
        size_t              count_;
        loff_t              moduleAdjust_;
        mutable addr_t      frameBase_;
        addr_t              unitBase_;
        const Function*     fun_; // function of scope
        const Symbol*       sym_;
    };
}



void EmitDebugSymbol::operator()(const Function& fun)
{
    assert(thread_);
    assert(fun.name() && *fun.name());

    // filter by name, if provided
    if (name_ && *name_)
    {
        if (strcmp(fun.name(), name_) != 0)
        {
            return;
        }
    }
    dbgout(1) << "----- " << fun.name() << " -----" << endl;

    const addr_t frameBase = CHKPTR(frame_)->frame_pointer();

    assert(!fun_);

    if (addr_t addr = fun.low_pc())
    {
        dbgout(1) << fun.name() << ": " << hex <<addr << dec << endl;

        addr += moduleAdjust_;
        SymbolMap* smap = CHKPTR(thread_->symbols());
        RefPtr<Symbol> sym = smap->lookup_symbol(addr);

        if (sym.get())
        {
            dbgout(1) << "sym=" << sym->name() << endl;
            if (!sym->name()->is_equal(name_))
            {
                return;
            }
            ++count_;
            TypeAdapter adapter(&reader_, thread_, frameBase, typeMap_);

            reader_.emit_function(*thread_, sym, events_, adapter, fun);
        }
    }
    else if (RefPtr<SharedString> name = fun.linkage_name())
    {
        count_ = reader_.lookup_function( thread_.get(),
                                          name->c_str(),
                                          frameBase,
                                          events_);
    }
}


/**
 * Check for a match with the given string, looking at both
 * mangled and unmangled names.
 */
static bool name_equals(const Dwarf::Datum& dat, const char* str)
{
    assert(str);

    string name;

    if (RefPtr<SharedString> linkageName = dat.linkage_name())
    {
        name = cplus_unmangle(linkageName->c_str());
    }

    if (name.empty())
    {
        return strcmp(dat.name(), str) == 0;
    }
    static const size_t N = sizeof(ANONYMOUS_NS_PREFIX) - 1;

    if (name.compare(0, N, ANONYMOUS_NS_PREFIX) == 0)
    {
        return name.substr(N + 2).compare(str) == 0;
    }
    return name.compare(str) == 0;
}


void EmitDebugSymbol::emit_symbol(

    const Dwarf::Datum&             dat,
    std::shared_ptr<Dwarf::Type>  type,
    addr_t                          val,
    addr_t                          frameBase,
    bool                            isRegister,
    bool                            isValue )
{
    assert(type);

    dbgout(1) << dat.name() << ", Dwarf::Type=" << type->name()
              << (isRegister ? " (register) " : " ");
    dbgout(1) << "val=0x" << hex << val << dec << endl;

    // According to the DWARF standard: In the case
    // of locations used for structure members, the
    // computation assumes that the base address of the
    // containing structure has been pushed on the
    // stack before evaluation the addressing operation.
    //
    addr_t addr = val;
    if (isRegister || addr == 0)
    {
        addr = frameBase;
    }

    TypeAdapter adapter(&reader_, thread_, addr, typeMap_);
    RefPtr<DataType> adaptedType = adapter.apply(type);

    if (!adaptedType)
    {
        throw logic_error(string("Could not adapt ") + typeid(*type).name());
    }
    RefPtr<SharedString> name(shared_string(dat.name()));

    if (isRegister)
    {
        dbgout(1) << dat.name() << "=" << val << endl;
        read_from_register(*adaptedType, val, *name);

        ++count_;
    }
    else
    {
        RefPtr<DebugSymbolImpl> sym;

        if (val)
        {
            if (isValue)
            {
                ostringstream buf;
                buf << val;
                sym = DebugSymbolImpl::create(*thread_, *adaptedType, buf.str(), name);
                sym->set_constant();
            }
            else
            {
                // note: val is the symbol's address
                sym = DebugSymbolImpl::create( 
                    &reader_,
                    *thread_,
                    *adaptedType,
                    *name,
                    val,
                    decl_file(fun_, dat).get(),
                    dat.decl_line());
            }
        }
        else
        {
            sym = const_value(*thread_, dat, *adaptedType, name.get());
        }
        if (sym)
        {
            ++count_;

            if (events_->notify(sym.get()))
            {
                sym->read(events_);
            }
            dbgout(2) << __func__ << ": " << sym->value() << endl;
        }
    }
}


/**
 * emit symbol
 */
void EmitDebugSymbol::operator()(const Dwarf::Datum& dat)
{
    assert(thread_);


    // skip unnamed objects
    if (dat.name() == 0 || *dat.name() == 0)
    {
        dbgout(1) << "unnamed, filtered out" << endl;
        return;
    }

    // Utils::dump_attributes(dat, clog);

    // filter by name, if name provided
    if (name_ && *name_ && !name_equals(dat, name_))
    {
        dbgout(1) << dat.name() << ": not matching " << name_ << endl;
        return;
    }
    if (!dat.loc())
    {
        if (std::shared_ptr<Die> die = dat.check_indirect(false))
        {
            // should never happen, but never say never ;)
            if (die->offset() != dat.offset())
            {
                return (*this)(dynamic_pointer_cast<Datum>(die));
            }
        }
    }
    dbgout(1) << "----- " << dat.name() << " -----" << endl;

    if (Dwarf_Addr startScope = dat.start_scope())
    {
        if (Symbol* fun = thread_current_function(thread_.get()))
        {
            if (fun->offset() < startScope)
            {
                dbgout(1) << "out of scope" << endl;
                return;
            }
        }
    }
    if (!events_)
    {
        return;
    }
    std::shared_ptr<Dwarf::Type> type = dat.type();
    if (!type)
    {
        cerr << "*** Warning: " << "datum with null type: " << dat.name() << endl;
        return;
    }
    addr_t val = 0;
    bool isRegister = false;
    bool isValue = false;

    dbgout(1) << "calculating frame_base" << endl;
    const addr_t frameBase = frame_base();
    dbgout(1) << "frameBase=" << hex << frameBase << dec << endl;

    if (std::shared_ptr<Dwarf::Location> loc = dat.loc())
    {
        dbgout(1) << "loc="<< loc << endl;
        const addr_t programCount = CHKPTR(frame_)->program_count();
        val = loc->eval(frameBase, moduleAdjust_, unitBase_, programCount);
        isValue = loc->is_value();
        isRegister = !isValue && loc->is_register(programCount, unitBase_);

        // experiment: deal with return-value-optimization,
        // location contains the address of a pointer to the variable
        if ((dat.access() == a_private) && !isValue)
        {
            size_t count = 0;
            thread_read(*thread_, val, val, &count);
        }
        dbgout(1) << "Val=0x" << hex << val << dec << " is_reg=" << isRegister << endl;
    }
    else // external variable?
    {
        dbgout(1) << "external? off=" << dat.cu_offset() << endl;

        SymbolMap* symbols = CHKPTR(thread_->symbols());

        SymbolEnum syms;
        RefPtr<SharedString> linkName = dat.linkage_name();

        if (linkName.is_null())
        {
            symbols->enum_symbols(dat.name(), &syms);
        }
        else
        {
            symbols->enum_symbols(linkName->c_str(), &syms, SymbolTable::LKUP_ISMANGLED);

            if (syms.empty())
            {
                static const SymbolTable::LookupMode mode =
                    (SymbolTable::LKUP_DYNAMIC | SymbolTable::LKUP_ISMANGLED);
                symbols->enum_symbols(linkName->c_str(), &syms, mode);
            }
        }
        //
        // todo: can there be several matches?
        //
        if (!syms.empty())
        {
            val = syms.front()->addr();
        }
    }

    if (type)
    {
        emit_symbol(dat, type, val, frameBase, isRegister, isValue);
    }
}


/**
 * @todo document
 */
void EmitDebugSymbol::read_from_register(
    DataType&       type,
    addr_t          val,
    SharedString&   name)
{
    ostringstream os;

    switch (CHKPTR(events_)->numeric_base(NULL))
    {
    case 16:  os << hex; break;
    case  8:  os << oct; break;
    }

    //hack: the DWARF ABI may use fpregs
    if (type.name()->is_equal("double"))
    {
        os << double(*reinterpret_cast<long double*>(val));
    }
    else
    {
        os << showbase << val;
    }
    // create a constant symbol
    RefPtr<DebugSymbolImpl> sym =
        DebugSymbolImpl::create(*CHKPTR(thread_),
                                type,
                                os.str(),
                                &name,
                                &reader_);

    events_->notify(sym.get());
    sym->read(events_);
}



template<typename T>
static void emit_var( Reader&           plugin,
                      EmitDebugSymbol&  emit,
                      const T&          func,
                      bool              paramsOnly )
{
    // emit the formal params
    const typename T::ParamList& p = func.params();
#if HAVE_LAMBDA_SUPPORT
    typedef typename T::ParamList::value_type V;
    for_each(p.begin(), p.end(), [&emit](const V& v) {
        emit(v);
    });
#else
    typedef typename T::ParamList::const_iterator Iter;

    for_each<Iter, EmitDebugSymbol&>(p.begin(), p.end(), emit);
#endif
    if (!paramsOnly)
    {
        plugin.enum_var(emit, func); // emit automatic vars
    }
}


/**
 * Map the program_count of a faked frame to the offset of
 * the DW_TAG_inlined_subroutine die that corresponds to it.
 */
typedef ext::hash_map<addr_t, Dwarf_Off> Pc2Off;


/**
 * @return the inlined subroutine description that corresponds
 * to given frame; if frame is a real frame, then return NULL
 */
/* experimental work in progress

static std::shared_ptr<InlinedInstance>
frame_to_inlined(Debug& dbg, const Pc2Off& pc2Off, addr_t addr)
{
    std::shared_ptr<InlinedInstance> inl;
    Pc2Off::const_iterator i = pc2Off.find(addr);
    if (i != pc2Off.end())
    {
        std::shared_ptr<Die> die = dbg.get_object(i->second, false, false);
        inl = dynamic_pointer_cast<InlinedInstance>(die);
        assert(inl);
    }
    return inl;
}
*/


#if (__GNUC__ >= 3)
namespace
{
    template<typename T>
    struct IterTraits
    {
        typedef typename T::value_type::value_type value_type;
        static inline const value_type& deref(const T& iter)
        {
            return **iter;
        }
    };

    template<typename T, typename Ptr, typename Ref>
    struct IterTraits<Dwarf::Iterator<T, Ptr, Ref> >
    {
        typedef T value_type;

        static inline const T& deref(std::shared_ptr<T> iter)
        {
            return *iter;
        }
    };
}


/**
 * @return the highest address of a parameter or variable used
 * in the given function.
 */
/*
template<typename T>
static inline addr_t frame_size(const T& func, addr_t unitBase, addr_t pc)
{
    addr_t addr = 0;
    typedef typename T::ParamList::const_iterator ParamIter;

    typename T::ParamList p(func.params());
    const ParamIter pend = p.end();
    for (ParamIter i = p.begin(); i != pend; ++i)
    {
        const typename IterTraits<ParamIter>::value_type& val =
            IterTraits<ParamIter>::deref(i);
        if (std::shared_ptr<Location> loc = val.loc(true))
        {
            addr_t a = loc->eval(0, 0, unitBase, pc);

            // clog << __func__ << ": " << val.name() << "=" << (void*)a << endl;
        }
    }

    typename T::VarList v = func.variables();
    typedef typename T::VarList::const_iterator VarIter;
    const VarIter vend = v.end();
    for (VarIter i = v.begin(); i != vend; ++i)
    {
        const typename IterTraits<VarIter>::value_type& val =
            IterTraits<VarIter>::deref(i);

        if (std::shared_ptr<Location> loc = val.loc(true))
        {
            addr_t a = loc->eval(0, 0, unitBase, pc);

            //clog << __func__ << ": " << val.name() << "=" << (void*)a << endl;
        }
    }

    return addr;
}
#else
template<typename T>
static inline addr_t
frame_size(const T& func, addr_t unitBase, addr_t pc)
{
    return 0;
}
*/
#endif

/*
static size_t frame_size(
    Debug&              dbg,
    const Pc2Off&       pc2Off,
    const StackTrace&   trace,
    Frame*              frame,
    addr_t              unitBase)
{
    assert(frame);
    size_t fsize = 0;

    for (;;)
    {
        size_t next = frame->index() + 1;
        if (next >= trace.size())
        {
            break;
        }
        addr_t addr = frame->program_count();
        if ((frame = trace.frame(next)) == NULL)
        {
            break;
        }

        // real frame?
        if (frame->real_program_count() == frame->program_count())
        {
            std::shared_ptr<Function> fun = dbg.lookup_function(addr);
            if (fun)
            {
                clog << "=== " << fun->name() << " ===\n";
                fsize = frame_size(*fun, unitBase, addr);
            }
        }
        else // it's a fake frame that corresponds to inlined function
        {

            if (std::shared_ptr<InlinedInstance> inl =
                frame_to_inlined(dbg, pc2Off, addr))
            {
                clog << "--- " << inl->function()->name() << " ---\n";
                fsize = frame_size(*inl, unitBase, addr);
            }
        }
    }

    return fsize;
}
*/


/**
 * @todo document
 */
static bool emit_inlined(
    Reader&             plugin,
    EmitDebugSymbol&    emit,
    bool                paramsOnly,
    Dwarf::Debug&       dbg,
    const Pc2Off&       pc2Off,
    const StackTrace&   trace,
    Frame&              frame)
{
    if (frame.real_program_count() != frame.program_count())
    {
        Pc2Off::const_iterator i = pc2Off.find(frame.program_count());
        if (i != pc2Off.end())
        {
            std::shared_ptr<Die> die = dbg.get_object(i->second, false, false);

            if (std::shared_ptr<InlinedInstance> inl = dynamic_pointer_cast<InlinedInstance>(die))
            {
            /*
                TODO: is it possible to compute locations reliably here?

                frame_size(dbg, pc2Off, trace, &frame, emit.unit_base());
                emit_var<List<Parameter> >(plugin, emit, *inl, paramsOnly);
             */
                return true;
            }
        }
    }
    return false;
}



size_t Reader::enum_locals(
    Thread*             thread,
    const char*         name,
    Frame*              frame,
    Symbol*             sym,
    DebugSymbolEvents*  events,
    bool                paramOnly)
{
    assert(thread);
    assert(sym);
    CHKPTR(frame);

    AddrOperationsContext addrCtxt(thread);
    size_t count = 0;

    if (std::shared_ptr<Function> fun = find_function(*CHKPTR(sym)))
    {
        dbgout(1) << sym->name() << ": fun=" << fun->name()
                  << " addr=" << hex << sym->addr() << dec << endl;

        if (name && events && fun->unit())
        {
            dbgout(1) << "enumerating macros..." << endl;
            MacroEmitter macro(*fun->unit(), *thread, *events, name, sym);
            if ((count = macro.enumerate()) != 0)
            {
                dbgout(1) << count << " macro(s) found: " << name << endl;
                return count;
            }
        }

        EmitDebugSymbol emitter(*this, thread, name, frame,
                                sym, events, fun.get());

        if (!emit_inlined(*this, emitter, paramOnly,
                            fun->owner(), pc2InlinedOff_,
                            *CHKPTR(thread->stack_trace()), *frame))
        {
            dbgout(1) << "emitting locals, frame=" << frame << endl;
            emit_var(*this, emitter, *fun, paramOnly);
        }

        count = emitter.count();
        dbgout(1) << "count=" << count << endl;
    }
    return count;
}


//
// todo: break into smaller functions: unit, module, etc.
//
size_t Reader::enum_globals(
    Thread*             thread,
    const char*         name,
    Symbol*             sym,
    DebugSymbolEvents*  events,
    LookupScope         scope,
    bool                enumFuncs)
{
    if (!sym)
    {
        return 0;
    }
    ZObjectScope objectScope;
    const SymbolTable* table = sym->table(&objectScope);
    if (!table) // marked for deletion?
    {
        return 0;
    }
    RefPtr<Process> process = table->process(&objectScope);

    dbgout(1) << __func__ << ": scope=" << scope << endl;
    Frame* stackFrame = thread_current_frame(CHKPTR(thread));
    if (!stackFrame)
    {
        clog << "*** Warning: " << __func__ << ": NULL stack frame\n";
        return 0;
    }

    AddrOperationsContext addrCtxt(thread);

    if (scope == LOOKUP_ALL) // look in all modules
    {
        size_t result = 0;
        RefPtr<SymbolMap> symbols = CHKPTR(thread->symbols());
        // iterate through all modules (i.e. main executable and DSOs)
        RefPtr<SymbolMap::LinkData> link = symbols->file_list();
        for (; link; link = link->next())
        {
            dbgout(1) << "enum_globals: " << link->filename() << endl;
            if (Handle dbg = get_debug_handle(link->filename(), process))
            {
                result += enum_globals(*thread,
                                        name,
                                        *sym,
                                        events,
                                        *dbg,
                                        *stackFrame,
                                        enumFuncs);
            }
        }
        return result;
    }
    else if (scope == LOOKUP_MODULE)
    {
        // look only in the current module
        if (Handle dbg = get_debug_handle(table->filename(), process))
        {
            return enum_globals(*thread,
                                name,
                                *sym,
                                events,
                                *dbg,
                                *stackFrame,
                                enumFuncs);
        }
    }
    else if (scope == LOOKUP_UNIT)
    {   // lookup inside the compilation unit
        if (Handle dbg = get_debug_handle(table->filename(), process))
        {
            return enum_unit_globals(*thread,
                                    name,
                                    *sym,
                                    events,
                                    *dbg,
                                    *stackFrame,
                                    enumFuncs);
        }
    }
    else
    {
        clog << "*** Warning: unhandled scope: " << scope <<  endl;
    }
    return 0;
}



/**
 * Emit functions in unit
 */
static void emit_funcs(EmitDebugSymbol& emit, const CompileUnit& unit)
{
    const FunList& fun = unit.functions();
    for (FunList::const_iterator i = fun.begin(); i != fun.end(); ++i)
    {
        if ((*i)->name() && *(*i)->name())
        {
            emit(**i);
        }
    }
}


/**
 * Enumerate global variables in given list of namespaces.
 */
static void enum_ns_globals
(
    const List<Namespace>& nsList,
    Dwarf::EmitDebugSymbol& emitter
)
{
    List<Namespace>::const_iterator i = nsList.begin();

    for (; i != nsList.end(); ++i)
    {
        const VarList& vars = i->variables();

        VarList::const_iterator v = vars.begin();
        for (; v != vars.end(); ++v)
        {
            emitter(*v);
        }
        // dig into nested namespaces
        enum_ns_globals(i->ns_list(), emitter);
    }
}


size_t Reader::enum_unit_globals(
    Thread&             thread,
    const char*         name,
    Symbol&             sym,
    DebugSymbolEvents*  events,
    Dwarf::Debug&       dbg,
    Frame&              stackFrame,
    bool                enumFuncs)
{
    size_t count = 0;

    if (std::shared_ptr<CompileUnit> unit =
        dbg.lookup_unit(stackFrame.program_count()))
    {
        EmitDebugSymbol emit(   *this,
                                &thread,
                                name,
                                &stackFrame,
                                &sym,
                                events);
        const VarList& vars = unit->variables();
    #ifndef HAVE_LAMBDA_SUPPORT
        // Note: predicate passed by reference
        // get the variables at compilation unit scope
        typedef VarList::const_iterator Iter;

        for_each<Iter, EmitDebugSymbol&>(vars.begin(), vars.end(), emit);
    #else
        for_each(vars.begin(), vars.end(), [&emit](const std::shared_ptr<Dwarf::Variable>& v) {
            emit(v);
            });
    #endif

        // dig into namespaces
        if (!emit.count())
        {
            enum_ns_globals(unit->namespaces(), emit);
            dbgout(1) << "enum_ns_globals=" << emit.count() << endl;
        }

        if (!emit.count() && name)
        {
            emit_funcs(emit, *unit);
        }
        count = emit.count();

        // no match? try macro definitions
        if (!count && name && events)
        {
            MacroEmitter macro(*unit, thread, *events, name, &sym);
            count = macro.enumerate();
        }
    }
    return count;
}


size_t Reader::enum_globals(
    Thread&             thread,
    const char*         name,
    Symbol&             sym,
    DebugSymbolEvents*  events,
    Dwarf::Debug&       dbg,
    Frame&              frame,
    bool                enumFuncs)
{
    size_t result = 0;
    const addr_t frameBase = frame.frame_pointer();
    Debug::Data data;

    for (string className; name && data.empty(); )
    {
        // functor that emits a debug symbol for each matching datum
        EmitDebugSymbol emit(*this, &thread, 0, &frame, &sym, events);

        // fetch the global variables that match the name
        data = dbg.lookup_global_data(name);

        if (data.empty())
        {
            string qualified = name;
            size_t n = qualified.rfind("::");
            // try again, using the unqualified name
            if (n != string::npos)
            {
                qualified = qualified.substr(n + 2);
                data = dbg.lookup_global_data(qualified.c_str());
            }
        }

        typedef Debug::Data::const_iterator Iter;
        for (Iter i = data.begin(); i != data.end(); ++i)
        {
            if (RefPtr<SharedString> mangledName = (*i)->linkage_name())
            {
                string tmp = cplus_unmangle(mangledName->c_str());
                if (tmp != name)
                {
                    continue;
                }
            }
            emit(*i);
        }
        result = emit.count();

        // The following is a work-around for when the
        // compiler does not generate a .debug_pubnames section:
        // look up the type, so that add_static_member is called;
        // the variable may be found in the static members cache.
        if (!result)
        {
            if (*name == 0 || !className.empty())
            {
                break;
            }
            className = name;
            size_t n = className.rfind("::");
            if (n != string::npos)
            {
                className = className.substr(0, n);
                // for nested classes and namespaces:
                n = className.rfind("::");
                if (n != string::npos)
                {
                    className = className.substr(n + 2);
                }
                if (className.empty())
                {
                    break;
                }
                Temporary<bool> autoFlag(shallow_, false);
                lookup_type(&thread, className.c_str(), frameBase, LOOKUP_MODULE);
            }
        }
    }
    if (!result)
    {
        result = enum_unit_globals( thread, name, sym,
                                    events, dbg, frame,
                                    enumFuncs);

        dbgout(1) << "enum_unit_globals=" << result << endl;
    }

    // to accomodate the expression interpreter, if a name was
    // specified but no symbol was found, look for functions;
    // function calls may be parts of an expression
    if (!result && name && enumFuncs)
    {
        result += lookup_function(&thread, name, frameBase, events);
    }
    return result;
}


static bool use_frame_handlers()
{
    static bool f = env::get_bool("ZERO_USE_FRAME_HANDLERS", false);
    return f;
}



unsigned Dwarf::expensive_type_lookup_level()
{
    static unsigned level = env::get("ZERO_EXPENSIVE_TYPE_LOOKUPS", 1);
    return level;
}



/**
 * Get the handle that corresponds to the module (shared object,
 * executable) where the program counter PC points to.
 */
static Handle get_handle(
    const Reader&        reader,
    const Thread&        thread,
    addr_t               pc,
    volatile loff_t&     adjustment)
{
    Handle handle;
    if (SymbolMap* symbols = thread.symbols())
    {
        if (SymbolTable* tbl = symbols->lookup_table(pc))
        {
            RefPtr<Process> process = thread.process();
            handle = reader.get_debug_handle(tbl->filename(), process);
            adjustment = tbl-> adjustment();
        }
    }
    return handle;
}



bool Reader::get_return_addr (
    Thread* thread,
    addr_t  addr,
    addr_t* retAddr
    ) const
{
    assert(thread);
    assert(retAddr);
    volatile loff_t adjust = 0;

    if (Handle dbg = get_handle(*this, *thread, addr, adjust))
    {
        assert_gte(addr, adjust);
        addr -= adjust;

      /***** FIXME: is this correct? *****/
        if (get_inlined_blocks(*dbg, addr))
        {
            InlinePtr inl = inlinedBlocks_.back();
            if (inl->high_pc() != addr)
            {
                *retAddr = inl->high_pc();
                return true;
            }
        }
      /***********************************/

        if (use_frame_handlers())
        {
            AddrOpsImpl addrOps(thread);
            RegTable regs;

            *retAddr = dbg->unwind_step(addr, addrOps, regs);
            if (*retAddr)
            {
                return true;
            }
        }
    }
    return false;
}



bool Reader::get_fun_range(
    Thread* thread,
    addr_t  addr,
    addr_t* low,
    addr_t* high
    ) const
{
    assert(thread);
    SymbolMap* symbols = CHKPTR(thread->symbols());
    SymbolTable* symtab = symbols->lookup_table(addr);
    if (!symtab)
    {
        return false;
    }
    RefPtr<SharedString> file = CHKPTR(symtab->filename());
    ZObjectScope scope;
    Handle dbg(get_debug_handle(file, symtab->process(&scope)));
    if (!dbg)
    {
        return false;
    }
    assert_gte(addr, symtab->adjustment());
    addr -= symtab->adjustment();

    // context for DWARF location calculations
    AddrOperationsContext addrCtxt(thread);
    std::shared_ptr<Function> fun(dbg->lookup_function(addr));
    if (fun.get())
    {
        if (low)
        {
            *low = fun->low_pc() + symtab->adjustment();
        }
        if (high)
        {
            *high = fun->high_pc() + symtab->adjustment();
        }
        return true;
    }
    return false;
}



DebugSymbol* Reader::get_return_symbol(
    Thread*         thread,
    const Symbol*   symbol, // symbol corresponding to function
    RefTracker*     tracker)
{
    assert(thread);
    assert(symbol);

    // context for DWARF location calculations
    AddrOperationsContext addrCtxt(thread);

    addr_t addr = 0;
    std::shared_ptr<Function> fun = find_function(*symbol, &addr);

    // If function not found, just return a NULL symbol; it is not
    // a serious error, maybe the debug info is not in DWARF format?
    if (!fun)
    {
        return 0;
    }
    //
    // TODO: handle inlined functions
    //
    RefPtr<DataType> retType;

    if (std::shared_ptr<Type> type = fun->ret_type())
    {
        TypeAdapter adapter(this, thread, addr, typeMap_);
        retType = adapter.apply(type);

        if (!retType)
        {
            string err = "Could not adapt ";
            throw logic_error(err + type->name());
        }
    }
    else
    {
        TypeSystem* types = CHKPTR(interface_cast<TypeSystem*>(thread));
        retType = CHKPTR(types)->get_void_type();
    }
    RefPtr<SharedString> name(shared_string(fun->name()));
    addr = thread->result();

    RefPtr<DebugSymbolImpl> debugSymbol =
        DebugSymbolImpl::create(this,
                                *thread,
                                *retType,
                                *name,
                                addr,
                                decl_file(fun.get(), *fun).get(),
                                fun->decl_line(),
                                true);
    if (tracker)
    {
        tracker->register_object(debugSymbol.get());
    }
    return debugSymbol.detach();
}



std::shared_ptr<Dwarf::Type> Reader::lookup_type_in_all_modules(
    RefPtr<Process> process,
    SymbolMap* symbols,
    const char* name,
    bool expensiveLookups)
{
    std::shared_ptr<Type> type;

    RefPtr<SymbolMap::LinkData> link = symbols->file_list();
    for (; link && !type; link = link->next())
    {
        if (Handle dbg = get_debug_handle(link->filename(), process))
        {
        #if DEBUG
            if (name)
            {
                dbgout(1) << "looking up " << name << " in " << dbg->filename() << endl;
            }
        #endif

            type = dbg->lookup_type(name, expensiveLookups);
        }
    }
    return type;
}



std::shared_ptr<Dwarf::Type> Reader::lookup_type(
    SymbolMap*      symbols,
    SymbolTable&    table,
    const char*     name,
    addr_t          addr,
    LookupScope     scope)
{
    std::shared_ptr<Dwarf::Type> type;
    const size_t thorough = expensive_type_lookup_level();

    if (name)
    {
        if (strncmp(name,
                    ANONYMOUS_NS_PREFIX,
                    sizeof(ANONYMOUS_NS_PREFIX) - 1) == 0)
        {
            // skip "anonymous_namespace::"
            name += sizeof(ANONYMOUS_NS_PREFIX) + 1;
        }
    }
    ZObjectScope objScope;
    if (Handle dbg = get_debug_handle(table.filename(), table.process(&objScope)))
    {
    #if DEBUG
        if (name)
        {
            dbgout(1) << "Looking up " << name << " in " << dbg->filename() << endl;
        }
    #endif
        if (scope == LOOKUP_UNIT)
        {
            addr_t pc = addr;
            if (table.is_loaded())
            {
                assert_gte(addr, table.adjustment());
                pc -= table.adjustment();
            }

            std::shared_ptr<CompileUnit> unit(dbg->lookup_unit(pc));
            if (unit)
            {
                type = Dwarf::lookup_type(unit->functions(), name);
            }
        }
        else if (scope == LOOKUP_MODULE)// lookup entire module
        {
            type = dbg->lookup_type(name, thorough);
        }
    }
    // look in all modules?
    if (!type && symbols && (scope == LOOKUP_ALL))
    {
        type = lookup_type_in_all_modules(table.process(&objScope), symbols, name, thorough);
    }
    return type;
}


/**
 * lookup_type entry point
 */
DataType* Reader::lookup_type(
    Thread*     thread,
    const char* name,
    addr_t      addr,
    LookupScope scope)
{
    assert(thread);
    assert(name);
    if (!thread || !name)
    {
        throw invalid_argument(__func__ + string(": null param"));
    }
    if (*name == 0)
    {
        return NULL;
    }
    if (RefPtr<DataType> cachedType = typeMap_.find(name))
    {
        dbgout(1) << "Found in typeMap: " << name << endl;
        return cachedType.detach();
    }

    ZObjectScope objectScope;
    SymbolTable* table = NULL;
    if (Frame* frame = thread_current_frame(thread))
    {
        if (frame->function())
        {
            table = frame->function()->table(&objectScope);
        }
        // if no address given, use the program counter at current frame
        if (addr == 0)
        {
            addr = frame->program_count();
        }
    }
    std::shared_ptr<Dwarf::Type> type;
    if (table)
    {
        SymbolMap* symbols = CHKPTR(thread->symbols());
        type = lookup_type(symbols, *table, name, addr, scope);
    }

    //
    // adapt Dwarf::Type
    //
    RefPtr<DataType> result;
    if (type)
    {
        AddrOperationsContext addrCtxt(thread);

        TypeAdapter adapter(this, thread, addr, typeMap_);
        adapter.set_depth(shallow_ ? ADAPT_SHALLOW : ADAPT_FULL);

        result = adapter.apply(type);

        if (!result || !result->name()->is_equal(name, true))
        {
            RefPtr<DataType> tmp = typeMap_.find(name);
            if (tmp)
            {
                result = tmp;
            }
        }
    }
    return result.detach();
}



TranslationUnit* Reader::lookup_unit_by_addr(Process* proc, SharedString* fname, addr_t addr)
{
    Handle dbg;
    if (fname)
    {
        dbg = get_debug_handle(fname, proc);
    }
    if (dbg)
    {
        if (std::shared_ptr<CompileUnit> unit = dbg->lookup_unit(addr))
        {
            return new Unit(dbg, unit, fname);
        }
    }
    return NULL;
}


static bool unit_matches(std::shared_ptr<CompileUnit> unit, const char* filename)
{
    assert(filename);
    RefPtr<SharedString> path = abspath(unit->full_path());

    if (path && filename && strcmp(path->c_str(), filename) == 0)
    {
        return true;
    }
    for (size_t i = 0; i < unit->source_files_count(); ++i)
    {
        path = CHKPTR(unit->filename_by_index(i));

        if (path->is_equal(filename)
         || canonical_path(path->c_str()) == filename)
        {
            return true;
        }
    }
    return false;
}


size_t Reader::lookup_unit_by_name (
    Process* proc,
    SharedString* moduleFileName,
    const char* filename,
    EnumCallback<TranslationUnit*, bool>* callback)
{
    size_t count = 0;

    if (Handle dbg = get_debug_handle(moduleFileName, proc))
    {
        const Debug::UnitList& units = dbg->units();

        for (Debug::UnitList::const_iterator i = units.begin();
                i != units.end();
                ++i)
        {
            if (!filename || unit_matches(*i, filename))
            {
                ++count;

                if (callback)
                {
                    RefPtr<TranslationUnit> unit =
                        new Unit(dbg, *i, moduleFileName);

                    if (!callback->notify(unit.get()))
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        dbgout(1) << __func__ << ": not found: " << moduleFileName->c_str() <<endl;
    }
    return count;
}



static size_t addr_to_line(
    const std::shared_ptr<CompileUnit>& unit,
    addr_t addr,
    addr_t* nearest,
    LineCallbackAdapter& lcba)
{
    size_t result = 0;
    Dwarf_Addr tmp = 0;

    if (nearest)
    {
        tmp = *nearest;
        result = unit->addr_to_line(addr, &tmp, &lcba);

        if (result)
        {
            assert(tmp);
            *nearest = tmp;
        }
    }
    else
    {
        result = unit->addr_to_line(addr, NULL, &lcba);
    }
    return result;
}


/**
 * Given an address, look up source file and line number.
 * The address is not translated, i.e. does not have
 * symtab->adjustment() added to it.
 */
size_t Reader::addr_to_line (
    const SymbolTable* symtab,
    addr_t  addr,
    addr_t* nearest,
    EnumCallback2<SharedString*, size_t>* events)
{
    assert(symtab);
    if (!symtab)
    {
        return 0;
    }
    ZObjectScope scope;
    Process* process = symtab->process(&scope);
    size_t result = 0;

    const addr_t adj = symtab->adjustment();
    const addr_t vaddr = addr + adj;

    //
    // obtain the handle for the module
    //
    if (Handle dbg = get_debug_handle(symtab->filename(), process))
    {
        //
        // lookup the compilation unit for the given address
        //
        if (std::shared_ptr<CompileUnit> unit = dbg->lookup_unit(addr))
        {
            LineCallbackAdapter lcba(adj, events, 0);

            result = ::addr_to_line(unit, addr, nearest, lcba);
        }

        if (!result)
        {
            dbgout(2) << "not found: "<< hex << vaddr << dec << endl;
        }
    }
    else
    {
        dbgout(2) << "not found: " << symtab->filename() << endl;
    }
    return result;
}



addr_t Reader::next_line_addr(
    const SymbolTable* symtab,
    addr_t addr,
    SharedString* srcFile,
    size_t srcLine
    ) const
{
    assert(symtab);
    assert(srcFile);

    if (!symtab || !srcFile)
    {
        return 0;
    }
    addr_t result = 0;
    dbgout(1) << __func__ << " " << hex << addr << dec << endl;

    ZObjectScope scope;
    RefPtr<SharedString> file = symtab->filename();
    RefPtr<Process> process = symtab->process(&scope);

    if (Handle dbg = get_debug_handle(file, process))
    {
        size_t next = 0;
        result = dbg->next_line(srcFile->c_str(),
                                srcLine,
                                addr,
                                &next);
    }
    return result;
}



size_t Reader::line_to_addr (
    Process*                process,
    SharedString*           module,
    loff_t                  adjust,
    SharedString*           sourceFile,
    size_t                  line,
    EnumCallback<addr_t>*   events,
    bool*                   cancelled)
{
    assert(module);
    assert(sourceFile);

    size_t count = 0;

    if (Handle dbg = get_debug_handle(module, process))
    {
        count = line_to_addr(*dbg, adjust,
                            sourceFile, line,
                            events, cancelled);

        dbgout(1) << module->c_str() << ": " << sourceFile << ":"
                  << line << " " << count << " matches" << endl;
    }
    else
    {
        dbgout(1) << "handle not found: " << module << endl;
    }
    return count;
}



size_t Reader::line_to_addr (
    Debug&                  dbg,
    loff_t                  adjust,
    SharedString*           sourceFile,
    size_t                  line,
    EnumCallback<addr_t>*   events,
    bool*                   cancelled)
{
    assert(sourceFile);

    LineCallbackAdapter lineEvents(adjust, NULL, events);

    if (!dbg.line_to_addr(sourceFile, line, lineEvents))
    {
        if (cancelled)
        {
            *cancelled = true;
        }
    }
    // return the count so far, even if cancelled
    return lineEvents.count();
}


/**
 * Match a Symbol table entry with the corresponding DWARF
 * function (DW_TAG_subprogram) entry
 */
std::shared_ptr<Dwarf::Function>
Reader::find_function(const Symbol& sym, addr_t* retAddr) const
{
    assert(get_addr_operations());

    ZObjectScope scope;
    std::shared_ptr<Dwarf::Function> fun;

    const SymbolTable* table = sym.table(&scope);
    if (!table)
    {
        return fun;
    }
    // use the value from the symbol table rather
    // than the memory-mapped address because the low_pc
    // and high_pc are expected to be relative.
    const addr_t addr = sym.value();

    if (retAddr)
    {
        *retAddr = addr + sym.offset();
    }
    RefPtr<SharedString> fileName = table->filename();
    if (Handle dbg = get_debug_handle(fileName, table->process(&scope)))
    {
        fun = dbg->lookup_function(addr);
    }
    else
    {
        dbgout(1) << "handle not found: " << fileName.get() << endl;
    }
    if (fun)
    {
        dbgout(1) << __func__ << '=' << fun->name() << endl;
    }
    else
    {
        dbgout(1) << __func__ << "=NULL addr=" << hex << addr << dec << endl;
    }
    if (!fun)
    {
        if (RefPtr<SharedString> mangledName = sym.name())
        {
            fun = get_fun_by_linkage_name(table->process(&scope), mangledName);
        }
    }
    return fun;
}



void Reader::enum_var(EmitDebugSymbol& emit, const Block& block)
{
    const Symbol* sym = emit.symbol();
    const addr_t pc = sym->value() + sym->offset();

    const VarList& v = block.variables();

    // emit variables in given block
    typedef VarList::const_iterator Iter;
    for (Iter i = v.begin(); i != v.end(); ++i)
    {
        emit(*i);
        // experiment: deal with return-value-optimization,
        // location contains the address of a pointer to the variable
        if ((*i)->access() == a_private)
        {
            // if a homonym variable follows, skip it
            Iter j = i;
            if (++j != v.end() && strcmp((*i)->name(), (*j)->name()) == 0)
            {
                i = j;
            }
        }
    }

    // now enumerate the variables inside the
    // inner (nested) lexical blocks, try blocks, etc.
    List<Block> blks = block.blocks();
    dbgout(1) << blks.size() << " block(s)" << endl;

    List<Block>::const_iterator i = blks.begin(), end = blks.end();
    for (; i != end; ++i)
    {
        if (pc >= i->low_pc() && pc < i->high_pc())
        {
            enum_var(emit, *i);
            break;
        }

    #ifdef DEBUG
        else
        {
            ZObjectScope scope;
            // report out-of-scope variables
            if (SymbolTable* table = sym->table(&scope))
            {
                const loff_t adjust = table->adjustment();

                dbgout(1) << __func__ << ": " << hex << pc
                          << " (" << pc + adjust
                          << ") not in scope [" << i->low_pc()
                          << ", " << i->high_pc() << dec << ")"
                          << endl;
            }
        }
    #endif // DEBUG
    }
}



/**
 * This function is invoked by enum_globals.
 * Purpose: support function identifiers in expressions
 * @todo include class methods if `this' is visible in scope
 */
size_t Reader::lookup_function (
    Thread*             thread,
    const char*         name,
    addr_t              frameBase,
    DebugSymbolEvents*  events)
{
    // pre-conditions
    assert(name);
    assert(thread);
    if (!name || !thread)
    {
        return 0;
    }

    size_t result = 0; // return the number of matches

    SymbolEnum funcs;

    // lookup symbol tables
    RefPtr<SymbolMap> symbols = CHKPTR(thread->symbols());
    //todo: part of this is duplicated in the engine core
    if (symbols->enum_symbols(name, &funcs) == 0)
    {
        symbols->enum_symbols(name, &funcs, SymbolTable::LKUP_DYNAMIC);
    }
    dbgout(1) << name << ": " << funcs.size() << " match(es)" << endl;
    assert(funcs.size() || result == 0);

    TypeAdapter adapter(this, thread, frameBase, typeMap_);

    // match functions from symbol table with Dwarf::Function
    // objects by their addresses
    FunctionEnum::const_iterator i = funcs.begin();
    for (; i != funcs.end(); ++i)
    {
        std::shared_ptr<Function> fun = find_function(**i);

        if (!fun)
        {
            fun = get_fun_by_linkage_name(thread->process(), name);
        }

        if (fun)
        {
            if (emit_function(*thread, *i, events, adapter, *fun))
            {
                ++result;
            }
        }
    }
    return result;
}



bool Reader::emit_function (
    Thread&                 thread,
    const RefPtr<Symbol>&   funSym,
    DebugSymbolEvents*      events,
    TypeAdapter&            adapter,
    const Function&         fun)
{
    assert(funSym);

    const addr_t addr = funSym->addr();
    assert(addr);

    RefPtr<DataType> type = adapter.get_type(fun);

    if (!type)
    {
        return false;
    }
#if 0 // DEBUG 
    else
    {
        clog << __func__ << ": ";
        clog << interface_cast<FunType&>(*type).return_type()->name() << endl;
    }
#endif
    if (events)
    {
        RefPtr<DebugSymbol> sym = DebugSymbolImpl::create(
            this,
            thread,
            *type,
            *funSym->demangled_name(false),
            addr,
            decl_file(&fun, fun).get(),
            fun.decl_line());

        dbgout(1) << type->name() << endl;

        events->notify(sym.get());
    }
    return true;
}



bool Reader::unwind_step (
    const Thread* thread,
    const Frame* frame, // current frame
    EnumCallback<Frame*>* callback)
{
    if (!fhandler_ || !CHKPTR(thread) || !CHKPTR(frame))
    {
        return false;
    }
    AddrOperationsContext context(thread);

    addr_t addr = frame->real_program_count();
    assert (addr);

    loff_t adjust = 0; // filled out by get_handle
    Handle dbg = get_handle(*this, *thread, addr, adjust);
    if (!dbg)
    {
        dbgout(1) << __func__ << ": .debug not found" << endl;
        return false;
    }
    //
    // create the illusion of stack frames for inlined funcs?
    //
    if (inline_heuristics())
    {
        RefPtr<Frame> f = fake_frame(thread, frame, *dbg, adjust);
        if (f.get())
        {
            if (callback)
            {
                callback->notify(f.get());
            }
            return true;
        }
    }
    RegTable frameRegs;

    frameRegs.regs[ABI::user_reg_fp()].value = frame->frame_pointer();
    frameRegs.regs[ABI::user_reg_fp()].state = RegTable::REG_READY;
    frameRegs.regs[ABI::user_reg_sp()].value = frame->stack_pointer();
    frameRegs.regs[ABI::user_reg_sp()].state = RegTable::REG_READY;
    //
    // linux-todo: what if the PC is up in the virtual kernel DSO?
    //
    frameRegs.regs[ABI::user_reg_pc()].value = addr;
    frameRegs.regs[ABI::user_reg_pc()].state = RegTable::REG_READY;

    assert_gte(addr, adjust);
    addr -= adjust; // make it relative to the module

    addr = dbg->unwind_step(addr, context.operations(), frameRegs);

    if (addr && addr != frame->real_program_count())
    {
        // unwind_step() post-conditions:
        assert(frameRegs.regs[ABI::user_reg_fp()].state == RegTable::REG_READY);
        assert(frameRegs.regs[ABI::user_reg_sp()].state == RegTable::REG_READY);

        RefPtr<FrameImpl> f = new FrameImpl;

        f->set_real_program_count(addr);
        f->set_program_count(addr);
        f->set_frame_pointer(frameRegs.regs[ABI::user_reg_fp()].value);
        f->set_stack_pointer(frameRegs.regs[ABI::user_reg_sp()].value);

        if (callback)
        {
            callback->notify(f.get());
        }
        return true;
    }
    else if (addr)
    {
        dbgout(1) << "addr=" << hex << addr << " frame real-addr="
                  << frame->real_program_count() << dec << endl;
    }
    return false;
}


namespace
{
    struct ZDK_LOCAL InlineFrameHelper : public EnumCallback<addr_t>
    {
        addr_t addr_;

        InlineFrameHelper() : addr_(0) { }

        void notify(addr_t addr) { if (!addr_) addr_ = addr; }
    };
}


/**
 * Helper called by fake_frame
 *
 * @note requires ZERO_INLINE_HEURISTICS
 */
bool Reader::get_inlined_blocks(Debug& debug, addr_t addr) const
{
    if (inlinedBlocks_.empty() || inlinedAddr_ != addr)
    {
        inlinedBlocks_.clear();
        if (std::shared_ptr<Function> fun = debug.lookup_function(addr))
        {
            if (!find_inlined_instance(*fun, addr))
            {
                return false;
            }
        }
    }
    assert(inlinedBlocks_.empty() || addr == inlinedAddr_);
    return !inlinedBlocks_.empty();
}


/**
* Use DW_TAG_inlined_subroutine info to create a fake
* frame in the stack trace.
*
* @note requires ZERO_INLINE_HEURISTICS
*/
RefPtr<Frame> Reader::fake_frame(
    const Thread*   thread,
    const Frame*    frame,  // current frame
    Debug&          dbg,
   loff_t          adjust)
{
    RefPtr<FrameImpl> result;

    addr_t addr = frame->real_program_count();
    assert(addr);

    assert_gte(addr, adjust);
    addr -= adjust;

    if (get_inlined_blocks(dbg, addr))
    {
        InlinePtr inlinedBlock = inlinedBlocks_.back();
        inlinedBlocks_.pop_back();

        // use the file and line of the call site to determine
        // the address of the call site (which is currently not
        // supplied as an attribute of DW_TAG_inlined_subroutine)
        if (size_t fileIndex = inlinedBlock->call_file())
        {
            if (std::shared_ptr<CompileUnit> unit = dbg.lookup_unit(addr))
            {
                --fileIndex;

                RefPtr<SharedString> file = unit->filename_by_index(fileIndex);
                size_t line = inlinedBlock->call_line();

                InlineFrameHelper fh;

                if (line_to_addr(dbg, adjust, file.get(), line, &fh))
                {
                    addr = fh.addr_;
                    // return a fake frame
                    RefPtr<FrameImpl> f = new FrameImpl;
                    f->set_program_count(addr);
                    assert(f->real_program_count() == addr);
                    f->set_stack_pointer(frame->stack_pointer());
                    f->set_frame_pointer(frame->frame_pointer());
                    addr_t realPC = frame->real_program_count();
                    f->set_real_program_count(realPC);

                    result = f;
                    pc2InlinedOff_[frame->program_count()] = inlinedBlock->offset();
                }
            }
        }
    }
    return result;
}


void Reader::add_fun_to_linkage_map(TypeSystem&, const Function& fun)
{
    if (RefPtr<SharedString> name = fun.linkage_name())
    {
        LinkageInfo info = { fun.owner().inode(), fun.offset() };
        linkageMap_.insert(make_pair(name, info));
    }
}


std::shared_ptr<Function>
Reader::get_fun_by_linkage_name(RefPtr<Process> proc, const char* name) const
{
    assert(name);
    if (TypeSystem* typeSystem = interface_cast<TypeSystem*>(proc.get()))
    {
        return get_fun_by_linkage_name(proc, typeSystem->get_string(name));
    }
    return get_fun_by_linkage_name(proc, shared_string(name));
}


std::shared_ptr<Function> Reader::get_fun_by_linkage_name(
    RefPtr<Process> process,
    const RefPtr<SharedString>& name
    ) const
{
    assert(name);
    std::shared_ptr<Function> fun;

    LinkageMap::const_iterator i = linkageMap_.find(name);
    if (i != linkageMap_.end())
    {
        if (Handle dbg = get_debug_handle(i->second.dbg_ino_, process))
        {
            Dwarf_Off off = i->second.off_;
            fun = dynamic_pointer_cast<Function>(dbg->get_object(off));
        }
    }
    return fun;
}


// Copyright (c) 2004, 2006, 2007, 2008  Cristian L. Vlasceanu
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
