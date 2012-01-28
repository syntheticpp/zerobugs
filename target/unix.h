#ifndef UNIX_H__98485A77_58F1_11DA_B82B_00C04F09BBCC
#define UNIX_H__98485A77_58F1_11DA_B82B_00C04F09BBCC
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
#include <set>
#include <vector>
#include "zdk/zero.h"
#include "zdk/zobject_impl.h"
#include "process.h"
#include "target.h"


/**
 * Base class for debugging targets on unix-like systems
 * such as Linux and FreeBSD
 */
class UnixTarget : public ZObjectImpl<Target>
{
protected:
    virtual ~UnixTarget() throw();

    explicit UnixTarget(debugger_type&);

    /**
     * @return level of verbosity of diagnostics messages
     * output to the standard logging device for debugging
     * the engine.
     */
    int verbose() const;

    /**
     * @return reference to the object debugging this target
     */
    virtual debugger_type& debugger() const { return dbg_; }

    /**
     * @return true if the debugger needs to stop all the
     * threads in the target in order to handle the current
     * event that occurred in the given thread.
     */
    virtual bool event_requires_stop(Thread*) { return false; }

    virtual void handle_event(Thread*);

    virtual RefPtr<SymbolMap> read_symbols() = 0;

    virtual TypeSystem* type_system() const
    { return types_.get(); }

    size_t word_size() const { return wordSize_; }
    void set_word_size(size_t wordSize) { wordSize_ = wordSize; }

    /**
     * default implementation, for convenience
     */
    bool pass_by_reg(Thread&, std::vector<RefPtr<Variant> >&)
    {
        return false;
    }

    virtual void add_thread_internal(const RefPtr<Thread>&) = 0;
    virtual bool remove_thread_internal(const RefPtr<Thread>&) = 0;

    void increment_iter_count() { ++iterCount_; }
    void decrement_iter_count();

    bool is_deletion_pending(const RefPtr<Thread>&) const;

    bool is_deletion_pending(pid_t) const;

    void reset_type_system();

    virtual RefPtr<ProcessImpl>
        new_process(pid_t, const ExecArg*, ProcessOrigin) = 0;


private:
    void manage_thread(const RefPtr<Thread>&);
    void unmanage_thread(const RefPtr<Thread>&);

public:
    /**
     * uses the template method pattern, calls new_process
     */
    void init_process(pid_t, const ExecArg*, ProcessOrigin,
                      const char* procName = NULL);

    /**
     * uses the template method pattern, calls read_symbols
     */
    void init_symbols(SymbolMap* = NULL);

    ProcessImpl* process() const { return proc_.get(); }

    SymbolMap* symbols() const { return symbols_.get(); }

    bool is_attached(Thread* = NULL) const;

    void add_thread(const RefPtr<Thread>&);
    void remove_thread(const RefPtr<Thread>&);

    //
    // implements the MemoryIO interface
    //
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

private:
    typedef std::set<RefPtr<Thread> > ThreadSet;
    typedef std::vector<RefPtr<Thread> > ThreadVector;

    debugger_type&          dbg_;
    RefPtr<ProcessImpl>     proc_;
    RefPtr<SymbolMap>       symbols_;
    ThreadSet               threads_;
    size_t                  wordSize_;
    RefPtr<TypeSystem>      types_;
    int                     iterCount_; // outstanding thread iterators

    // keep track of pending additions and deletions
    ThreadVector            threadsToAdd_;
    ThreadVector            threadsToDelete_;
};

#endif // UNIX_H__98485A77_58F1_11DA_B82B_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
