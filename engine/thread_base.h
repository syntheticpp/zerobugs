#ifndef THREAD_COMMON_H__D07EF7C0_EEC7_4571_AA3B_7B2D2073735F
#define THREAD_COMMON_H__D07EF7C0_EEC7_4571_AA3B_7B2D2073735F
//
// $Id: thread_base.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#ifdef HAVE_SYS_PARAM_H
 #include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
 #include <sys/user.h>
#endif
#include <string>
#include "zdk/config.h"
#include "zdk/ref_counted_impl.h"
#include "zdk/weak_ptr.h"
#include "zdk/zero.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "target/syscall_pending.h"
#include "target/target.h"


/**
 * Factors out some implementation details common to
 * ThreadImpl and CoreThreadImpl.
 */
class ThreadBase : public RefCountedImpl<Thread>
{
    // for set/get_user_data set/get_user_object
    typedef ext::hash_map<std::string, word_t> UserDataMap;
    typedef ext::hash_map<std::string, RefPtr<ZObject> > UserObjectMap;

    ThreadBase(const ThreadBase&); // non-copyable
    ThreadBase& operator=(const ThreadBase&); // non-assignable

protected:
    ThreadBase() : traceable_(true) { }
    ~ThreadBase() throw();

public:
    void set_user_data(const char*, word_t, bool);

    bool get_user_data(const char*, word_t*) const;

    void set_user_object(const char*, ZObject*, bool = true);

    ZObject* get_user_object(const char*) const;

    size_t enum_user_regs(EnumCallback<Register*>*);
    size_t enum_fpxregs(EnumCallback<Register*>*);

    virtual long syscall_num() const;

    virtual word_t result() const;
    virtual int64_t result64() const;

    virtual long double result_double(size_t) const;

    virtual Process* process() const;

    /**
     * @return the Debugger attached to this thread
     */
    Debugger* debugger() const;

    /**
     * @return the thread's map of symbol tables
     */
    SymbolMap* symbols() const;

    virtual DebugRegs* debug_regs();

    virtual ZObject* regs(ZObject* reserved = NULL) const;

    /**
     * @return an object holding the floation-point register contents;
     * the actual object definition is opaque and designed to be
     * used internally by the implementation
     */
    virtual ZObject* fpu_regs(ZObject* reserved = NULL) const;

    virtual addr_t program_count() const;
    virtual addr_t frame_pointer() const;
    virtual addr_t stack_pointer() const;

    virtual reg_t read_register(int, bool = false) const;

    bool is_32_bit() const;

    bool is_syscall_pending(addr_t pc = 0) const;

    bool is_traceable() const { return traceable_; }
    void set_traceable(bool traceable) { traceable_ = traceable; }

    void cancel_syscall() { thread_cancel_syscall(*this); }

    void clear_cached_regs() { regs_.reset(), fpuRegs_.reset(); }

    pid_t get_signal_sender() const { return -1; } // unknown

    // Memory IO
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

    virtual const char* name() const;

private:
    mutable RefPtr<ZObject> regs_;
    mutable RefPtr<ZObject> fpuRegs_;
    UserDataMap             userData_;
    UserObjectMap           userObj_;
    bool                    traceable_;
    mutable std::string     name_;
};

#endif // THREAD_COMMON_H__D07EF7C0_EEC7_4571_AA3B_7B2D2073735F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
