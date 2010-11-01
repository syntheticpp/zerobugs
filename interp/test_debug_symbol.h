#ifndef TEST_DEBUG_SYMBOL_H__4D707A77_6057_4E9C_9C20_D42CD47FBF59
#define TEST_DEBUG_SYMBOL_H__4D707A77_6057_4E9C_9C20_D42CD47FBF59
//
// $Id: test_debug_symbol.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/debug_sym.h"
#include "zdk/debug_symbol_list.h"

class TestDebugSymbol : public RefCountedImpl<DebugSymbol>
{
BEGIN_INTERFACE_MAP(TestDebugSymbol)
    INTERFACE_ENTRY(DebugSymbol)
END_INTERFACE_MAP()

    typedef DebugSymbolList Children;


    TestDebugSymbol(const TestDebugSymbol&);

public:
    TestDebugSymbol(DataType& type, const char* name, const char* value);

    virtual ~TestDebugSymbol() throw() {};

    /**
     * The name of the variable represented by this symbol
     */
    virtual SharedString* name() const;

    virtual DataType* type() const;

    virtual addr_t addr() const { return addr_; }

    /**
     * For aggregated symbols such as classes, structs etc.
     */
    virtual DebugSymbol* parent() const { return NULL; }

    /**
     * Enumerate the member data of variables of aggregate types
     * (classes, unions, arrays, etc), calling the provided callback
     * (if not NULL) for every child.
     * @return the number of children
     */
    virtual size_t enum_children(DebugSymbolCallback*) const;

    /**
     * @return the n-th child
     * @throw out_of_range
     */
    virtual DebugSymbol* child(size_t n) const;

    /**
     * The symbol's value, formatted as a string.
     */
    virtual SharedString* value() const;

    /**
     * Indicates whether this debug symbol corresponds to a value
     * returned from a function (as opposed to a parameter or variable)
     */
    virtual bool is_return_value() const { return false; }

    virtual int compare(const DebugSymbol*) const;

    /**
     * The thread context in which the variable is read.
     */
    virtual Thread* thread() const { return NULL; }

    virtual size_t depth() const { return 1; }

    virtual void add_child(DebugSymbol*);

    /**
     * Updates the current value of the symbol by reading it
     * from the debugged thread's memory.
     */
    virtual void read(DebugSymbolEvents*, long reserved = 0);

    /**
     * Return the reader object that produced this symbol
     */
    virtual DebugInfoReader* reader() const { return NULL; }

    /**
     * For bitfields, return the offset inside the byte where
     * the symbol starts
     */
    ///// virtual size_t bit_offset() const = 0;

    /**
     * Parse the given string according to this symbol's type,
     * and convert it to a value of that type, then write the
     * value to the symbol's location in memory (or in a CPU
     * register, in some cases).
     * @return the number of parsed characters.
     * @note if the resulting number of parsed characters is
     * not equal to the length of the string, the input string
     * is treated as erroneous and the memory remains unchanged.
     */
    virtual size_t write(const char* str);

    virtual DebugSymbol* clone(SharedString*, DataType*, bool) const;

    virtual const char* producer() const { return "test"; }

    virtual bool is_constant() const { return constant_; }
    virtual DebugSymbol* nth_child(size_t n)
    {
        assert(children_.size() > n);
        return children_[n].get();
    }
    /**
     * @return line number in source file where declared, if
     * the information is available, otherwise return 0.
     */
    virtual size_t decl_line() const { return 0; }

    /**
     * @return source file name where declared, if
     * the information is available, otherwise return NULL.
     */
    virtual SharedString* decl_file() const { return 0; }

    SharedString* type_name() const
    {
        if (DataType* t = type())
        {
            return t->name();
        }
        return NULL;
    }
    const char* tooltip() const { return ""; }

private:
    RefPtr<DataType> type_;

    addr_t addr_;

    RefPtr<SharedString> name_;

    RefPtr<SharedString> value_;

    bool constant_;

    Children children_;
};
#endif // TEST_DEBUG_SYMBOL_H__4D707A77_6057_4E9C_9C20_D42CD47FBF59
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
