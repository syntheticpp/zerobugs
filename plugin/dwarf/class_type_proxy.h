#ifndef CLASS_TYPE_PROXY_H__43B677E5_AEF4_473B_B376_055EEBA787BE
#define CLASS_TYPE_PROXY_H__43B677E5_AEF4_473B_B376_055EEBA787BE
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

#include "zdk/check_ptr.h"
#include "zdk/types.h"
#include "zdk/observer_impl.h"
#include "typez/public/data_type_impl.h"
#include "typez/public/types.h"
#include "dwarfz/public/interface.h"


namespace Dwarf
{
    class KlassType;
    class Reader;

    /**
     * Base for ClassTypeProxy -- saves one name_ member per instance;
     * uses lesser memory than the default NamedTypeImpl base.
     */
    CLASS ProxyBase
    {
        SubjectImpl<> subject_;

    protected:
        explicit ProxyBase(const char* = 0) {}

        void attach_impl(Observer* observer)
        {
            subject_.attach(CHKPTR(observer));
        }

        SharedString* get_name() const { return 0; }

        void notify_state_change() { subject_.notify_state_change(); }
    };

    /*
     * In the DWARF debug format, addresses may be computed
     * at runtime as specified by a set of rules in the Location
     * object. The calculations are relative to a base address,
     * which, in the case of a class component (base class or
     * member data) it is assumed to be the start address of
     * the most-derived object that contains the component.
     * Now, if the user provides an incorrect base address when
     * reading a debug symbol that corresponds to an object of
     * this class, the calculations will be wrong; we don't
     * want to memorize incorrect results in the TypeMap cache.
     * An other reason for using this proxy is to lazily adapt
     * the class type and thus improve performance.
     */
    CLASS ClassTypeProxy : public DataTypeImpl<ClassType, ProxyBase>
                         , public TypeChangeObserver
    {
        typedef DataTypeImpl<ClassType, ProxyBase> BaseType;

    public:
        DECLARE_UUID("d05b0960-649a-44a2-b346-ee6896a7523b")

        BEGIN_INTERFACE_MAP(ClassTypeProxy)
            INTERFACE_ENTRY(ClassTypeProxy)
            INTERFACE_ENTRY(TypeChangeObserver)
            INTERFACE_ENTRY_INHERIT(BaseType)
            INTERFACE_ENTRY_DELEGATE(klass_)
        END_INTERFACE_MAP()

        ClassTypeProxy(
            Reader&,            // debug info reader
            const RefPtr<Thread>&,
            const KlassType&,   // Dwarf type
            ClassType&);        // adapted type

        ~ClassTypeProxy() throw() { }

        // ----- Observer Interface -----
        void on_state_change(Subject*);

    protected:
        // ----- DataType Interface -----
        SharedString* name() const;

        size_t bit_size() const;

        bool is_fundamental() const;

        bool is_equal(const DataType* other) const;

        int compare(const char*, const char*) const;

        size_t parse(const char*, Unknown2*) const;

        void describe(int fd) const;

        SharedString* make_pointer_name(const char*, RefTracker*) const;

        SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

        // ----- ClassType Interface -----
        virtual SharedString* unqualified_name() const;

        virtual bool has_vtable(Thread*) const;

        virtual bool is_union() const;

        ///@return number of base classes
        virtual size_t base_count() const;

        ///@return n-th base
        ///@throw out_of_range
        virtual const BaseClass* base(size_t) const;

        /// lookup base class by name, if offset is
        /// not null, fill out the offset of the base
        /// class in the derived
        virtual const BaseClass* lookup_base(
            const SharedString* name,
            off_t* offset = NULL,
            bool  recursive = false) const;

        ///@return number of virtual bases
        virtual size_t virtual_base_count() const;

        ///@return number of data members
        virtual size_t member_count() const;

        ///@return n-th member
        ///@throw out_of_range
        virtual const Member* member(size_t n) const;

        ///@return number of methods (member functions)
        virtual size_t method_count() const;

        ///@return n-th method
        ///@throw out_of_range
        virtual const Method* method(size_t) const;

        virtual RTTI* rtti(Thread*) const;

        size_t
        enum_template_type_param(EnumCallback<TemplateTypeParam*>*) const;

        size_t
        enum_template_value_param(EnumCallback<TemplateValueParam*>*) const;

        // ----- TypeChangeObserver -----
        virtual bool on_type_change(DebugSymbol*, DataType*);

    protected:
        void check_shallow(const char*, Thread*) const;

        bool is_adaptation_needed(const DebugSymbol&) const;

    private:
        void adapt_deep(Thread*, addr_t = 0) const;

        typedef RefPtr<ClassType> ClassTypePtr;

        mutable Reader*         reader_;

        mutable ClassTypePtr    klass_;
        mutable bool            shallow_;
        mutable addr_t          addr_;
        ino_t                   inode_;
        Dwarf_Off               offset_;
        Dwarf_Off               declOffset_;
        RefPtr<Observer>        observ_;
    };
}
#endif // CLASS_TYPE_PROXY_H__43B677E5_AEF4_473B_B376_055EEBA787BE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
