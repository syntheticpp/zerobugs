#ifndef BREAKPOINT_IMG_H__A9069932_BEDD_4B6F_931D_58F653B10306
#define BREAKPOINT_IMG_H__A9069932_BEDD_4B6F_931D_58F653B10306
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
#include "zdk/persistent.h"

/**
 * A class for saving and restoring user breakpoints.
 * Models the "image" of a breakpoint that gets persisted on disk.
 */
class ZDK_LOCAL BreakPointImage : public RefCountedImpl<>, public Persistent
{
public:
    DECLARE_UUID("b94cae13-d509-443b-b41a-2e41ae74819e")

    BEGIN_INTERFACE_MAP(BreakPointImage)
        INTERFACE_ENTRY(BreakPointImage)
        INTERFACE_ENTRY_INHERIT(Persistent)
    END_INTERFACE_MAP()

    explicit BreakPointImage(volatile BreakPoint&);

    BreakPointImage();

    virtual ~BreakPointImage() throw();

    addr_t addr() const { return addr_; }

    void set_addr(addr_t);

    bool is_enabled() const { return enabled_ > 0; }

    SharedString* symbol_name() const { return symName_.get(); }

    SharedString* file() const { return file_.get(); }

    word_t line() const { return line_; }

    const std::string& condition() const { return condition_; }

    word_t activation_counter() const;

    bool auto_reset() const;

    BreakPoint::Type type() const { return type_; }

    /// @return the task ID of the thread that was the
    /// current execution thread when the breakpoint
    /// was set
    /// @note it is not necessarily the thread to which
    /// the breakpoint applies -- if type is GLOBAL, then
    /// it applies to all threads.
    pid_t origin_lwpid() const { return lwpid_; }

    bool is_deferred() const { return deferred_ > 0; }

protected:
    /*** InputStreamEvents ***/
    void on_word(const char*, word_t);

    void on_string(const char*, const char*);

    /*** Streamable ***/
    uuidref_t uuid() const { return _uuid(); }

    size_t write(OutputStream*) const;

private:
    addr_t                  addr_;
    word_t                  enabled_;
    word_t                  deferred_;
    RefPtr<SharedString>    symName_;
    RefPtr<SharedString>    file_;
    word_t                  line_;
    word_t                  activationCount_;
    pid_t                   lwpid_;   // process where originally set
    std::string             condition_;
    BreakPoint::Type        type_;
};
#endif // BREAKPOINT_IMG_H__A9069932_BEDD_4B6F_931D_58F653B10306
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
