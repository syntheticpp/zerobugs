#ifndef PROCESS_H__07BB282D_ACDB_430D_A3CA_5EE581304216
#define PROCESS_H__07BB282D_ACDB_430D_A3CA_5EE581304216
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

#include <string>
#include "zdk/process.h"
#include "zdk/type_system.h"
#include "zdk/weak_ptr.h"
#include "zdk/watch.h"
#include "zdk/zobject_impl.h"
#include "dharma/sarray.h"


class ProcessImpl : public ZObjectImpl<Process>
{
    ProcessImpl& operator=(const ProcessImpl&);
    ProcessImpl(const ProcessImpl&);

public:
DECLARE_UUID("152b85af-c406-4a1c-a0d8-3102c2add27b")

BEGIN_INTERFACE_MAP(ProcessImpl)
    INTERFACE_ENTRY(Process)
    INTERFACE_ENTRY(ProcessImpl)
    INTERFACE_ENTRY_AGGREGATE(type_system())
    INTERFACE_ENTRY_DELEGATE(watches_.ref_ptr())
END_INTERFACE_MAP()

    static bool init();

    virtual ~ProcessImpl() throw();

    ProcessImpl(Target&,
                pid_t pid,
                ProcessOrigin,
                const std::string* cmdline = NULL,
                const char* filename = NULL);

    virtual pid_t pid() const { return pid_; }

    virtual const char* name() const;
    void set_name(const char*);

    virtual SharedString* command_line() const;

    virtual ProcessOrigin origin() const { return origin_; }

    virtual SymbolMap* symbols() const;

    virtual size_t enum_modules(EnumCallback<Module*>*);

    virtual size_t enum_threads(EnumCallback<Thread*>*);

    virtual const char* const* environment() const;
    void set_environment(const char* const*);

    virtual Thread* get_thread(pid_t tid, unsigned long);

    virtual BreakPointManager* breakpoint_manager() const;

    virtual bool is_attached(Thread* thread) const;

    virtual SymbolTable* vdso_symbol_tables() const;

    virtual Debugger* debugger() const;

    void set_pid(pid_t pid) { pid_ = pid; }

    Target* target() const;

    /**
     * Read the cmdline from the /proc filesystem
     */
    void read_cmdline() const;

    /*** implements the MemoryIO interface ***/
    virtual void read_data(
        addr_t  address,
        word_t* buffer,
        size_t  howManyWords,
        size_t* wordsRead = 0) const;

    virtual void write_data(addr_t, const word_t*, size_t);

    virtual void read_code(
        addr_t  address,
        word_t* buffer,
        size_t  howManyWords,
        size_t* wordsRead = 0) const;

    virtual void write_code(addr_t, const word_t*, size_t);

    void set_watches(const RefPtr<WatchList>& watches);

protected:
    TypeSystem* type_system() const;

private:
    WeakPtr<Target>     target_;
    pid_t               pid_;

    const ProcessOrigin origin_;        // how was it started?
    mutable SArray      environ_;

    mutable RefPtr<SharedString> name_; // filename
    mutable RefPtr<SharedString> cmdline_;
    WeakPtr<WatchList>  watches_;
};


#endif // PROCESS_H__07BB282D_ACDB_430D_A3CA_5EE581304216
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
