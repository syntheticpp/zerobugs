#ifndef READER_H__C229CF16_4EF0_4F92_A3E0_47F146A2F6EE
#define READER_H__C229CF16_4EF0_4F92_A3E0_47F146A2F6EE
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
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include "dwarfz/public/interface.h"
#include "zdk/frame_handler.h"
#include "zdk/zero.h"
#include "zdk/debug_sym.h"
#include "zdk/version_info_impl.h"
#include "debug_cache.h"
#include "handle.h"
#include "type_map.h"

#undef get_return_symbol

class FunctionEnum;


namespace Dwarf
{
    class Block;
    class Debug;
    class EmitDebugSymbol;
    class Function;
    class InlinedInstance;
    class TypeAdapter;

    /**
     * A plugin that implmenents the DebugInfoReader
     * interface, for the DWARF format.
     */
    CLASS Reader
        : public DebuggerPlugin
        , public DebugInfoReader
        , public FrameHandler
        , public UnitHeadersCallback
        , VersionInfoImpl<ZERO_API_MAJOR, ZERO_API_MINOR, 67>
        , boost::noncopyable
    {
        // disable copy and assignment
        Reader(const Reader&);
        Reader& operator=(const Reader&);

    public:
        DECLARE_UUID("ccd27d0d-c18f-48f4-90ea-36cdef5c0af5")

        Reader();

        virtual ~Reader() throw ();

        TypeMap& type_map() { return typeMap_; }

        size_t lookup_function(
            Thread*         thread,
            const char*     name,
            addr_t          addrBase,// for location eval
            DebugSymbolEvents*);

        bool emit_function(
            Thread&,
            const RefPtr<Symbol>&,
            DebugSymbolEvents*,
            TypeAdapter&,
            const Function&);

    private:
        DESCRIPTION("DWARF Reader")

    BEGIN_INTERFACE_MAP(Reader)
        INTERFACE_ENTRY(Reader)
        INTERFACE_ENTRY(DebuggerPlugin)
        INTERFACE_ENTRY(DebugInfoReader)
        INTERFACE_ENTRY(FrameHandler)
        INTERFACE_ENTRY(VersionInfo)
    END_INTERFACE_MAP()

        const char* copyright() const;

        //
        // Plugin interface
        //
        void release();

        //
        // DebuggerPlugin interface
        //
        bool initialize(Debugger*, int* argc, char*** argv);

        void start() { } // no explicit start needed
        void shutdown();

        void register_streamable_objects(ObjectFactory*);

        void on_table_init(SymbolTable*);
        void on_table_done(SymbolTable*);

        void on_attach(Thread*);
        void on_detach(Thread*);

        bool on_event(Thread*, EventType);

        /**
         * Plug-in is notified that the debugger is about
         * to resume all threads in the debugged program.
         */
        void on_program_resumed() {};

        void on_insert_breakpoint(volatile BreakPoint*) {}
        void on_remove_breakpoint(volatile BreakPoint*) {}

        void on_syscall(Thread*, int sysCallNum) {}

        bool on_progress(const char*, double, word_t) { return true; }

        bool on_message(const char*, Debugger::MessageType, Thread*, bool)
        {
            return false;
        }

        //
        // DebugInfoReader interface
        //
        const char* format() const { return "dwarf"; }

        size_t enum_locals(
            Thread* thread,
            const char* name, // NULL or empty matches all
            Frame* frame,
            Symbol* func,
            DebugSymbolEvents*,
            bool paramOnly);

        size_t enum_globals(
            Thread* thread,
            const char* name,
            Symbol* func,
            DebugSymbolEvents*,
            LookupScope,
            bool enumFuncs);

        size_t addr_to_line(
            const SymbolTable*,
            addr_t,
            addr_t*,
            EnumCallback2<SharedString*, size_t>*);

        addr_t next_line_addr(  const SymbolTable*,
                                addr_t addr,
                                SharedString* file,
                                size_t line) const;

        size_t line_to_addr(
            Process*        process,
            SharedString*   moduleName,
            loff_t          moduleAdjust,
            SharedString*   sourceFile,
            size_t          lineNumber,
            EnumCallback<addr_t>*,
            bool*           cancelled = NULL);

        size_t line_to_addr(
            Debug&          dbg,
            loff_t          moduleAdjust,
            SharedString*   sourceFile,
            size_t          lineNumber,
            EnumCallback<addr_t>*,
            bool*           cancelled = NULL);

        bool get_return_addr(Thread*, addr_t, addr_t*) const;

        DebugSymbol* get_return_symbol(
            Thread*,
            const Symbol*,
            ENFORCE_REF_PTR_);

        /**
         * Lookup the function that contains the given address.
         */
        bool get_fun_range(Thread*, addr_t, addr_t*, addr_t*) const;

        DataType* lookup_type(Thread*, const char*, addr_t, LookupScope);

        /**
         * lookup type information in all modules
         */
        boost::shared_ptr<Dwarf::Type> lookup_type_in_all_modules(
            RefPtr<Process>,
            SymbolMap*,
            const char* typeName,
            bool useExpensiveLookups);

        boost::shared_ptr<Dwarf::Type> lookup_type(
            SymbolMap* symbols,
            SymbolTable& table,
            const char* typeName,
            addr_t addr,
            LookupScope);

        /**
         * init FrameHandler, nothing to do
         */
        void init(const Thread*) { }

        /**
         * FrameHandler
         * @return true on success, false otherwise
         */
        bool unwind_step(const Thread*, const Frame*, EnumCallback<Frame*>*);

        /**
         * Use DW_TAG_inlined_subroutine info to create a fake
         * frame in the stack trace.
         */
        RefPtr<Frame> fake_frame(const Thread*, const Frame*, Debug&, loff_t);

        TranslationUnit* lookup_unit_by_addr(Process*, SharedString*, addr_t);

        size_t lookup_unit_by_name(
                Process*,
                SharedString*,
                const char*,
                EnumCallback<TranslationUnit*, bool>*);

        /**
         * Given an ELF symbol, find the corresponding
         * Dwarf::Function object, so that its params
         * and variables can be enumerated.
         */
        boost::shared_ptr<Function> find_function(const Symbol&, addr_t* = 0) const;

        void ensure_debug_cache_inited() const;

    public:
        /**
         * enumerate variables in a lexical block.
         */
        void enum_var(EmitDebugSymbol&, const Block&);

        Handle get_debug_handle(ino_t, const RefPtr<Process>&) const;
        Handle get_debug_handle(SharedString*, const RefPtr<Process>&) const;

        Handle get_debug_handle(
            const RefPtr<SharedString>& file,
            const RefPtr<Process>&      process) const
        {
            return get_debug_handle(file.get(), process);
        }

        boost::shared_ptr<Function>
            get_fun_by_linkage_name(RefPtr<Process>,
                                    const char*) const;
        boost::shared_ptr<Function>
            get_fun_by_linkage_name(RefPtr<Process>,
                                    const RefPtr<SharedString>&) const;

        void add_fun_to_linkage_map(TypeSystem&, const Dwarf::Function&);

        Debugger* debugger() const { return debugger_; }

        boost::shared_ptr<Type> db_lookup_type(
            const RefPtr<Process>&,
            const char* typeName) const;

    private:
        // helpers
        size_t enum_globals(
            Thread&             thread,
            const char*         name,
            Symbol&             func,
            DebugSymbolEvents*  events,
            Dwarf::Debug&       dbg,
            Frame&              stackFrame,
            bool                enumFuncs);

        size_t enum_unit_globals(
            Thread&             thread,
            const char*         name,
            Symbol&             sym,
            DebugSymbolEvents*  events,
            Dwarf::Debug&       dbg,
            Frame&              stackFrame,
            bool                enumFuncs);

        bool find_inlined_instance(const Block& block, addr_t addr) const;

        bool get_inlined_blocks(Debug& dbg, addr_t addr) const;

        // UnitHeadersCallback
        void query_unit_headers(const std::string&, std::vector<Dwarf_Unsigned>&) const;

    private:
        struct LinkageInfo
        {
            ino_t dbg_ino_;
            Dwarf_Off off_;
        };
        typedef ext::hash_map<RefPtr<SharedString>, LinkageInfo> LinkageMap;

        // map linkage names to Die offset information
        typedef std::vector<boost::shared_ptr<InlinedInstance> > InlineStack;

        Debugger*                           debugger_;
        mutable std::auto_ptr<DebugCache>   handleCache_;
        mutable TypeMap                     typeMap_;
        bool                                shallow_;
        bool                                attached_;
        bool                                fhandler_; // enable frame handler?

        // data for inline support (mutable so that
        // get_inlined_blocks be called from const methods)
        mutable addr_t                      inlinedAddr_;
        mutable InlineStack                 inlinedBlocks_;
        ext::hash_map<addr_t, Dwarf_Off>    pc2InlinedOff_;

        LinkageMap                          linkageMap_;
        mutable Mutex                       mutex_;
    };


    /**
     * @return true if ZERO_USE_EXPENSIVE_LOOKUPS is set in the environment.
     * If true, attempt to find the full type definition, rather than just
     * being happy with a forward declaration that matches.
     * If a full definition is not found in the current module, lookup all
     * program modules.
     */
    ZDK_LOCAL unsigned expensive_type_lookup_level();


} // namespace Dwarf


#endif // READER_H__C229CF16_4EF0_4F92_A3E0_47F146A2F6EE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
