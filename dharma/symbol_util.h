#ifndef SYMBOL_UTIL_H__1060362520
#define SYMBOL_UTIL_H__1060362520
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

#include <cassert>
#include <iosfwd>
#include <string>
#include <vector>
#include <set>
#include "zdk/debug_sym.h"
#include "zdk/enum.h"
#include "zdk/ref_ptr.h"
#include "zdk/symbol.h"
#include "unmangle/unmangle.h"


/* Should this be moved to the symbolz library? */

/**
 * "stock" callback functor, accumulates the enumerated
 * symbols into a vector.
 */
class ZDK_LOCAL SymbolEnum : public EnumCallback<Symbol*>
{
public:
    typedef std::vector<RefPtr<Symbol> > container_type;
    typedef container_type::const_iterator const_iterator;

    virtual ~SymbolEnum() {}
    SymbolEnum() {}

    const_iterator begin() const { return symbols_.begin(); }
    const_iterator end() const { return symbols_.end(); }

    RefPtr<Symbol> front() const { return symbols_.front(); }
    RefPtr<Symbol> back() const { return symbols_.back(); }

    bool empty() const { return symbols_.empty(); }
    size_t size() const { return symbols_.size(); }

    void clear() { symbols_.clear(); }

    RefPtr<Symbol> operator[](size_t i) const
    {
        return symbols_[i];
    }

    long _count() const { return symbols_.size(); }

    void notify(Symbol* symbol);

private:
    container_type symbols_;
    std::set<addr_t> uniqueAddrs_;
};


/**
 * Filters out symbols that are not functions.
 */
class ZDK_LOCAL FunctionEnum : public SymbolEnum
{
protected:
    virtual void notify(Symbol* symbol)
    {
        assert(symbol);

        if (symbol->is_function())
        {
            SymbolEnum::notify(symbol);
        }
    }
};


/**
 * Same as SymbolEnum, only it acts upon debug symbols
 */
class ZDK_LOCAL DebugSymbolEnum : public DebugSymbolEvents
{
public:
    typedef std::vector<RefPtr<DebugSymbol> > container_type;
    typedef container_type::const_iterator const_iterator;

    virtual ~DebugSymbolEnum() {}
    DebugSymbolEnum() {}

BEGIN_INTERFACE_MAP(DebugSymbolEnum)
    INTERFACE_ENTRY(DebugSymbolEvents)
END_INTERFACE_MAP()

    const_iterator begin() const { return symbols_.begin(); }
    const_iterator end() const { return symbols_.end(); }

    size_t size() const { return symbols_.size(); }

private:
    bool notify(DebugSymbol* symbol)
    {
        assert(symbol);
        symbols_.push_back(RefPtr<DebugSymbol>(symbol));
        return false;
    }

    bool is_expanding(DebugSymbol*) const { return false; }

    int numeric_base(const DebugSymbol*) const { return 0; }

    void symbol_change(DebugSymbol*, DebugSymbol*) { }

private:
    container_type symbols_;
};


////////////////////////////////////////////////////////////////
RefPtr<DebugSymbol>
apply_transform(DebugSymbol&, DebugSymbol*, DebugSymbolEvents*);


void
print_symbol(std::ostream&, const Symbol&);

void
print_symbol(std::ostream&, addr_t, RefPtr<Symbol>, bool mod = false);


inline std::string demangle(const char* name, bool showParams = true)
{
    int flags = showParams ? UNMANGLE_DEFAULT : UNMANGLE_NOFUNARGS;
    return cplus_unmangle(name, flags);
}


inline std::string
demangle(const std::string& name, bool showParams = true)
{
    int flags = showParams ? UNMANGLE_DEFAULT : UNMANGLE_NOFUNARGS;
    return cplus_unmangle(name, flags);
}

inline std::ostream& operator<<(std::ostream& os, RefPtr<Symbol> sym)
{
    print_symbol(os, sym ? sym->addr() : 0, sym);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Symbol& sym)
{
    print_symbol(os, sym);
    return os;
}

#endif // SYMBOL_UTIL_H__1060362520
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
