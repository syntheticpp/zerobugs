#ifndef SYMBOL_TABLE_IMPL_H__A52780F9_8B27_4126_A157_9A824FBF25FD
#define SYMBOL_TABLE_IMPL_H__A52780F9_8B27_4126_A157_9A824FBF25FD
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
#include "zdk/config.h"
#include "zdk/zobject_impl.h"
#include "symbolz/private/async_fun.h"
#include "symbolz/private/predicates.h"
#include "symbolz/private/symbol_impl.h"
#include "symbolz/private/symbol_table_base.h"
#include "dharma/object_manager.h"
#include "dharma/task_pool.h"
#include "generic/singleton.h"
#include <memory>
#include <vector>
#include <boost/utility.hpp>

#ifdef DEBUG_OBJECT_LEAKS
 #define DECLARE_IMPL(c, b) CLASS c : public b, public CountedInstance<c>
#else
 #define DECLARE_IMPL(c, b) CLASS c : public b
#endif

namespace ELF
{
    class Binary;
    class SymbolTable;
}

class TypeSystem;
typedef Singleton<TaskPool> SymbolTaskPool;



/**
 * Implement the SymbolTable interface
 */
DECLARE_IMPL(SymbolTableImpl, SymbolTableBase)
{
public:
    DECLARE_UUID("d59be504-3e83-49dc-bd73-9be0a88e0261")
BEGIN_INTERFACE_MAP(SymbolTableImpl)
    INTERFACE_ENTRY(SymbolTableImpl)
    INTERFACE_ENTRY_INHERIT(SymbolTableBase)
END_INTERFACE_MAP()

    typedef RefPtr<SymbolTableImpl> Ptr;
    typedef RefPtr<SharedString> StringPtr;

    typedef RefPtr<Symbol> SymbolPtr;
    typedef std::vector<SymbolPtr> SymbolList;
    typedef std::vector<SymAddrPtr> SymAddrList;

    template<typename Pred>
    struct SymbolsByName : public SymbolList
    {
        typedef Pred predicate;

        void sort()
        {
            std::sort(this->begin(), this->end(), Pred());
        }
        void trim() // trim extra memory
        {
            SymbolsByName(*this).swap(*this);
        }
    };
    typedef SymbolsByName<Predicate::CompareDemangled> SymbolsByDemangledName;

    static Ptr read_tables( Process*,
                            RefPtr<SharedString>,
                            const ELF::Binary&,
                            SymbolTableEvents&,
                            addr_t vaddr,
                            addr_t upper = 0,
                            bool notifyOnDone = true);

protected:
    /**
     * Construct a SymbolTable out of an ELF symbol table.
     */
    SymbolTableImpl
      (
        Process*,
        StringPtr filename,
        const ELF::SymbolTable&,
        SymbolTableEvents&,
        addr_t vmemAddr,   // where loaded in memory
        addr_t loadAddr,   // preferred addr spec'd in ELF hdr
        addr_t upper
      );

    void read(const ELF::SymbolTable&, bool notifyOnDone = true);

    bool is_lazy_stub() const { return index_ == -1; }

public:
    /**
     * Construct a lazy-loading table
     */
    SymbolTableImpl
      (
        Process*,
        const std::string& filename,
        SymbolTableEvents&,
        addr_t vmemAddr,
        addr_t upper
      );

    virtual ~SymbolTableImpl() throw();

    /**
     * ELF section name
     */
    virtual SharedString* name() const;

    virtual SharedString* filename(bool = false) const;

    virtual bool is_loaded() const;

    virtual bool is_dynamic() const;

    /**
     * @return the number of symbols in the table.
     */
    virtual size_t size() const;

    Symbol* lookup_symbol(addr_t) const;

    size_t enum_addresses_by_line( SharedString* file,
                                   size_t line,
                                   EnumCallback<addr_t>*
                                 ) const;

    size_t enum_symbols(const char*, EnumCallback<Symbol*>*, LookupMode) const;

    SymbolTable* next() const;
    SymbolTableImpl* next_impl() const;

    bool is_virtual_shared_object() const { return virtual_; }

    void set_is_virtual_shared_object()
    {
        virtual_ = true;
        for (Ptr tbl = next_impl(); tbl; tbl = tbl->next_impl())
        {
            tbl->virtual_ = true;
        }
    }

    void sort_by_addr() const;
    void sort_by_name() const;

    void ensure_loaded() const;

    void enum_all_symbols(EnumCallback<Symbol*>*) const;

private:
    bool lazy_load(const char* func, bool warn = false) const;

    SymAddrPtr create_symbol(const ELF::Symbol&, const char*, TypeSystem*);

    void sort_by_addr_and_name_async();
    void sort_by_demangled_name() const;

    void wait_for_async_sort() const;

    ///@note called from lazy_load, hence const
    void set_realname(const RefPtr<SharedString>&) const;

private:
    typedef AsyncFun<SymbolTableImpl> Delegate;

    mutable StringPtr   name_;      // this table's name
    StringPtr		    filename_;	// executable or DSO
    bool                dynamic_;
    bool                virtual_;
    int8_t              index_;

    mutable SymAddrList symbols_;

    mutable StringPtr realname_;
    mutable SymbolsByName<Predicate::CompareName> symbolsByName_;

    /**
     * @note sort_by_demangled_name may be called from
     * enum_symbols, which is const
     */
    mutable SymbolsByDemangledName symbolsByDemangledName_;

    // async delegates
    // use the TaskPool to asynchronously sort symbols
    boost::shared_ptr<Delegate> sortByAddr_;
    boost::shared_ptr<Delegate> sortByName_;
    boost::shared_ptr<Delegate> sortByDemangledName_;
};


std::auto_ptr<ELF::Binary> get_symbol_tables_binary(const char*);

#undef DECLARE_IMPL
#endif // SYMBOL_TABLE_IMPL_H__A52780F9_8B27_4126_A157_9A824FBF25FD
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
