#ifndef reader_H__70E93067_9421_4550_8C81_B35381DC19F6
#define reader_H__70E93067_9421_4550_8C81_B35381DC19F6
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

#include "zdk/zero.h"
#include "zdk/debug_sym.h"
#include "zdk/version_info_impl.h"
#include "stabz/public/descriptor.h"

#undef get_return_symbol


namespace Stab
{
typedef google::dense_hash_map<
    RefPtr<SharedString>, RefPtr<Descriptor> > DescriptorMap;


/**
 * Reads debug information in the STABS format.
 */
class Reader
    : public DebuggerPlugin
    , public DebugInfoReader
    , public Stab::Events
    , VersionInfoImpl<ZERO_API_MAJOR, ZERO_API_MINOR, 13>
{
public:
    Reader();

    DESCRIPTION("STABS Reader")

BEGIN_INTERFACE_MAP(Reader)
    INTERFACE_ENTRY(DebuggerPlugin)
    INTERFACE_ENTRY(DebugInfoReader)
    INTERFACE_ENTRY(VersionInfo)
END_INTERFACE_MAP()

protected:

    virtual ~Reader() throw();

    virtual void release();

    // --- DebuggerPlugin interface
    /**
     * Pass pointer to debugger and the command line params
     * to plug-in modules.
     */
    virtual bool initialize(Debugger*, int* argc, char*** argv);

    virtual void start() { } // no explicit start needed
    virtual void shutdown();

    virtual void register_streamable_objects(ObjectFactory*);

    /**
     * This method is called whenever the debugger
     * is about to read a new symbol table.
     */
    virtual void on_table_init(SymbolTable*);

    /**
     * Called when the debugger has finished reading
     * a symbol table.
     */
    virtual void on_table_done(SymbolTable*);

    /**
     * Called when the debugger attaches itself to a new thread
     */
    virtual void on_attach(Thread*);

    /**
     * Called for each thread that finishes, and with a
     * NULL thread when the debugger detaches from the
     * program.
     */
    virtual void on_detach(Thread*);

    /**
     * Notification sent to plug-ins when the debuggee stops.
     * The plug-in is given the opportunity to take over
     * handling the user break point. If this method returns
     * true, the notification is not passed around to other
     * plug-ing, and the internal handling is skipped.
     * This way, a plug-in that implements a GUI can take
     * control over the user interaction.
     */
    virtual bool on_event(Thread*, EventType);

    /**
     * Plug-in is notified that the debugger is about to resume
     * all threads in the debugged program.
     */
    virtual void on_program_resumed();

    virtual void on_insert_breakpoint(volatile BreakPoint*);
    virtual void on_remove_breakpoint(volatile BreakPoint*);

    virtual void on_syscall(Thread*, int sysCallNum) {};

    bool on_message(const char*, Debugger::MessageType, Thread*, bool)
    { return false; }

    /**
     * When a plug-in calls Debugger::progress, the
     * notification is being passed to all the loaded
     * plug-ins. The initiator may choose to ignore the
     * event if it identifies the cookie as being the
     * same value as passed to the progress() call.
     */
    virtual bool on_progress(
        const char* what,
        double percent,
        word_t cookie);

    // --- DebugInfoReader interface
    virtual const char* format() const { return "stabs"; }

    /**
     * Enumerates the data objects local to
     * a given thread and function. The frame base
     * for the function is also specified. If the
     * events sink pointer is not null, then its
     * notify method is invoked for every DebugSymbol
     * that corresponds to a local. A local datum may
     * be a function parameter a variable, or a constant.
     * The count of locals is returned.
     */
    virtual size_t enum_locals(
        Thread* thread,
        const char* name,
        Frame* frame,
        Symbol* func,
        DebugSymbolEvents*,
        bool paramOnly);

    virtual size_t enum_globals(
        Thread* thread,
        const char* name,
        Symbol* func,
        DebugSymbolEvents* events,
        LookupScope,
        bool enumFunctions);

    /**
     * @return true if the source file and line number
     * for the given address are found, and call the
     * specified EnumCallback2 interface (if not NULL)
     */
    virtual size_t addr_to_line(
        const SymbolTable*,
        addr_t,
        addr_t*,
        EnumCallback2<SharedString*, size_t>*);

    /**
     * @return the first address generated for the
     * source line following the line where given
     * symbol is defined (supports the debugger's
     * `next' command).
     */

    virtual addr_t next_line_addr(  const SymbolTable*,
                                    addr_t addr,
                                    SharedString* file,
                                    size_t line) const;

    /**
     * Enumerate addresses generated for given source line.
     * @return address count
     */
    virtual size_t line_to_addr(
        Process*                process,
        SharedString*           moduleName,
        loff_t                  moduleAdjust,
        SharedString*           sourceFile,
        size_t                  lineNumber,
        EnumCallback<addr_t>*   observer,
        bool*                   cancelled = NULL);

    bool get_return_addr(Thread*, addr_t, addr_t*) const;

    DebugSymbol* get_return_symbol(Thread*, const Symbol*, ENFORCE_REF_PTR_);

    bool get_fun_range(Thread*, addr_t, addr_t*, addr_t*) const;

    DataType* lookup_type(Thread*, const char*, addr_t, LookupScope);

    TranslationUnit* lookup_unit_by_addr(Process*, SharedString*, addr_t);

    size_t lookup_unit_by_name(
            Process*,
            SharedString*,
            const char*,
            EnumCallback<TranslationUnit*, bool>*);

    // --- Stab::Events interface
    void on_section(const char*);
    void on_begin(SharedString&, const char*, size_t);

    bool on_stab(size_t, const stab_t&, const char*, size_t);
    void on_done(size_t);

protected:
    RefPtr<Descriptor> get_descriptor(SharedString*) const;

    RefPtr<Descriptor> get_descriptor(SharedString*, TypeSystem*);

    RefPtr<Descriptor> get_descriptor(SharedString*, Thread* thread);

    void reset_progress_info();

    size_t enum_functions(
        CompileUnit&        unit,
        Thread&             thread,
        const char*         name,
        DebugSymbolEvents*  events);

    size_t enum_globals(
        Thread& thread,
        const char* name,
        Symbol& func,
        DebugSymbolEvents* events,
        CompileUnit&,
        bool enumFunctions);

    size_t enum_globals(
        Thread& thread,
        const char* name,
        Symbol& func,
        DebugSymbolEvents* events,
        Descriptor&,
        bool enumFunctions);

    void ensure_parsed(Thread&, Descriptor&);
    void ensure_parsed(Thread&, Descriptor&) const;

    void ensure_parsed(Thread&, CompileUnit&);
    void ensure_parsed(Thread&, CompileUnit&) const;

private:
    Reader(const Reader&);
    Reader& operator=(const Reader&);

    Debugger*       debugger_;
    DescriptorMap   descriptors_;

    // --- for progress info
    RefPtr<SharedString> file_;
    double          percent_;
    size_t          numEntries_;
    std::string     section_;
    std::string     progress_;

    bool            lazy_;
    bool            showProgress_;
    bool            attached_;
};

}// namespace Stab


#endif // reader_H__70E93067_9421_4550_8C81_B35381DC19F6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
