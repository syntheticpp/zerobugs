#ifndef DEBUG_SYMBOL_WRAP_H__93633106_43FB_4DA6_80B5_23B8346FBE37
#define DEBUG_SYMBOL_WRAP_H__93633106_43FB_4DA6_80B5_23B8346FBE37
//
// $Id: debug_sym_wrap.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/python.hpp>
#include "zdk/debug_sym.h"
#include "zdk/zobject_impl.h"
#include "zdk/weak_ptr.h"
#include "interp/context.h"


class DebugSymbolEventsProxy : public DebugSymbolEvents
{
    DebugSymbolEvents* events_;
    int numericBase_;

public:
    BEGIN_INTERFACE_MAP(DebugSymbolEventsProxy)
        INTERFACE_ENTRY(DebugSymbolEvents)
    END_INTERFACE_MAP()

    explicit DebugSymbolEventsProxy(DebugSymbolEvents* events)
        : events_(events), numericBase_(-1)
    { }

    /**
     * Notified when a new symbol (possibly the child of an
     * aggregated object such a class instance or array) is
     * detected; if the method returns true, the symbol's
     * value is read.
     */
    virtual bool notify(DebugSymbol* dsym)
    {
        return events_ ? events_->notify(dsym) : false;
    }

    /**
     * Symbols that correspond to aggregate objects such as
     * class instances or arrays may be expanded, so that the
     * user can inspect their sub-parts. This method is called
     * by the reader implementations to determine if the client
     * wants such an aggregate object to be expanded or not.
     */
    virtual bool is_expanding(DebugSymbol* dsym) const
    {
        return events_ ? events_->is_expanding(dsym) : false;
    }

    /**
     * Readers call this method to determine what numeric base
     * should be used for the representation of integer values.
     */
    virtual int numeric_base(const DebugSymbol* dsym) const
    {
        if (numericBase_ >= 0)
        {
            return numericBase_;
        }
        return events_ ? events_->numeric_base(dsym) : 0;
    }
    void set_numeric_base(int numericBase)
    {
        if (numericBase != 0 && numericBase != 10 && numericBase != 16)
        {
            numericBase = -1;
        }
        numericBase_ = numericBase;
    }

    /**
     * A change in the symbol has occurred (name, type, address etc.)
     * A pointer to the old values is passed in.
     */
    virtual void symbol_change(DebugSymbol* newSym, DebugSymbol* old)
    {
        if (events_)
        {
            events_->symbol_change(newSym, old);
        }
    }
};



class ZDK_LOCAL DebugSymbolWrap : public ZObjectImpl<>
{
public:
    template<typename T>
    explicit DebugSymbolWrap(T sym, DebugSymbolEvents* events)
        : sym_(sym)
        , childrenRead_(false)
        , hasContext_(false)
        , expanded_(false)
        , events_(events)
        , context_(interface_cast<Context*>(events))
    {
        hasContext_ = context_;
    }

    ~DebugSymbolWrap() throw();

    addr_t addr() const { return sym_ ? sym_->addr() : 0; }

    const char* name() const;

    const char* value() const;

    void set_value(const char*);

    void set_constant();

    bool is_constant() const { return sym_ ? sym_->is_constant() : false; }

    boost::python::object type() const;

    void add_child(RefPtr<DebugSymbolWrap>);

    boost::python::list children() const;

    bool has_children() const;

    bool is_expanding() const
    {
        if (sym_)
        {
            if (DebugSymbolEvents* e = events())
            {
                return e->is_expanding(sym_.get());
            }
        }
        return false;
    }

    /**
     * @return the thread that owns this symbol
     */
    boost::python::object thread() const;

    /**
     * @return the process that owns the thread that owns this symbol
     */
    boost::python::object process() const;

    boost::python::object type_system() const;

    void read();

    //void set_type_name(const char*);
    void set_type_name(DataType*);

    const char* type_name() const;

    void set_tooltip(const char*);
    const char* tooltip() const;

    /**
     * Create a new debug symbol in the current thread space
     */
    RefPtr<DebugSymbolWrap> create(const char* name, DataType*, addr_t);

    DebugSymbol* detach() { return sym_.detach(); }

    DebugSymbolEvents* events() const { return &events_; }

    void set_numeric_base(int numericBase);

    // for GUI clients that want to remember the state of a treeview
    bool is_expanded() const { return expanded_; }
    void set_expanded(bool expanded) { expanded_ = expanded; }

private:
    void read_on_main_thread();

    RefPtr<DebugSymbol> sym_;

    mutable bool childrenRead_ : 1;
    bool hasContext_ : 1;
    bool expanded_ : 1;
    mutable boost::python::list children_;
    mutable DebugSymbolEventsProxy events_;
    RefPtr<Context> context_;
};

#endif // DEBUG_SYMBOL_WRAP_H__93633106_43FB_4DA6_80B5_23B8346FBE37
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
