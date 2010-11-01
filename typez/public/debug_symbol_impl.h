#ifndef DEBUG_SYMBOL_IMPL_H__1B0A180A_9FBD_4967_B570_56C45ED36905
#define DEBUG_SYMBOL_IMPL_H__1B0A180A_9FBD_4967_B570_56C45ED36905
//
// $Id: debug_symbol_impl.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "types.h"
#include "typez/public/debug_symbol_collection.h"


CLASS DebugSymbolImpl
    : public RefCountedImpl<DebugSymbol>
#ifdef DEBUG_OBJECT_LEAKS
    , public CountedInstance<DebugSymbolImpl>
#endif
{
public:
    typedef std::auto_ptr<DebugSymbolCollection> DebugChildrenPtr;

    DECLARE_UUID("4cc166be-2a2c-4c3d-9a75-9534506fc649")

BEGIN_INTERFACE_MAP(DebugSymbolImpl)
    INTERFACE_ENTRY(DebugSymbolImpl) // for upcasting internally
    INTERFACE_ENTRY(DebugSymbol)
    INTERFACE_ENTRY_AGGREGATE(method())
END_INTERFACE_MAP()

    DebugSymbolImpl(const DebugSymbolImpl&);

protected:
    /**
     * @todo: document
     */
    DebugSymbolImpl
      (
        DebugInfoReader*,
        Thread&,
        DataType&,
        SharedString&,
        addr_t,
        SharedString* declFile,
        size_t declLine,
        bool isReturnValue = false
      );

    DebugSymbolImpl
      (
        Thread&,
        DataType&,
        const std::string&,
        RefPtr<SharedString> name,
        DebugInfoReader* = NULL,
        Method* method = NULL
      );

public:
    static RefPtr<DebugSymbolImpl> create
      (
        DebugInfoReader*,
        Thread&,
        DataType&,
        SharedString&,
        addr_t,
        SharedString* declFile = NULL,
        size_t declLine = 0,
        bool isReturnValue = false
      );

    /**
     * creates a CONSTANT debug symbol
     */
    static RefPtr<DebugSymbolImpl> create
      (
        Thread&,
        DataType&,
        const std::string&,
        RefPtr<SharedString> name = RefPtr<SharedString>(),
        DebugInfoReader* = 0,
        Method* = NULL
      );

    virtual ~DebugSymbolImpl() throw();

    DebugSymbol* parent() const
    {
        return parent_.ref_ptr().get();
    }
    void set_tentative_parent(DebugSymbol* sym)
    {
        parent_ = sym;
        depth_  = 0;
    }

    void detach_from_parent() { parent_ = 0; }

    // todo: deprecate the bitmask
    void read(DebugSymbolEvents*, long bitmask = 0);

    void add_child(DebugSymbol* child);

    void remove_all_children();

    /**
     * Thread where the variable represented by
     * this debug symbol is defined.
     */
    Thread* thread() const;

    SharedString* name() const;

    virtual void set_name(SharedString* name);

    SharedString* type_name() const;

    SharedString* value() const;

    void set_value(const RefPtr<SharedString>& value)
    {
        value_ = value;
    }

    bool is_constant() const { return constant_; }

    void set_constant() { constant_ = true; }

    bool is_return_value() const { return retValue_; }

    int compare(const DebugSymbol*) const;

    /**
     * How many level from top parent to this
     * object (for struct and class members)?
     */
    size_t depth() const;

    addr_t addr() const { return addr_; }

    void set_addr(addr_t addr) { addr_ = addr; }

    /**
     * Notify the callback object, if not null, for each child.
     * @return number of children.
     */
    size_t enum_children(DebugSymbolCallback*) const;

    DebugSymbol* nth_child(size_t n);

    DataType* type() const;

    bool is_fundamental_type() const;

    DebugInfoReader* reader() const { return reader_; }

    bool set_type(DataType& type);

    size_t write(const char*);

    DebugSymbol* clone( SharedString* value,
                        DataType*,
                        bool isConst
                      ) const;

    void set_method(Method* method);

    size_t decl_line() const { return declLine_; }

    SharedString* decl_file() const { return declFile_.get(); }

    const DebugChildrenPtr& children() const { return children_; }

    void set_children(std::auto_ptr<DebugSymbolCollection>);

    void add_child_impl(DebugSymbolImpl* child);

    void set_type_name(SharedString* name) { typeName_ = name; }

    const char* tooltip() const;
    virtual void set_tooltip(const char*);

private:
    // non-assignable
    DebugSymbolImpl& operator=(const DebugSymbolImpl&);

    void read_bit_field(DebugSymbolEvents*, bitsize_t mask);

    RefPtr<Method> method() const;

    void ensure_children();

private:
    DebugInfoReader* const  reader_;
    WeakPtr<DebugSymbol>    parent_;
    WeakPtr<Thread>         thread_;
    WeakDataTypePtr         type_;

    RefPtr<SharedString>    name_;
    RefPtr<SharedString>    value_;
    addr_t                  addr_;

    mutable uint16_t        depth_;
    // true if this symbol corresponds to a return value
    bool                    retValue_ : 4;

    bool                    constant_ : 4;
    uint32_t                declLine_;
    RefPtr<SharedString>    declFile_;
    DebugChildrenPtr        children_;
    RefPtr<SharedString>    typeName_;
    RefPtr<SharedString>    tooltip_;
};

#endif // DEBUG_SYMBOL_IMPL_H__1B0A180A_9FBD_4967_B570_56C45ED36905
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
