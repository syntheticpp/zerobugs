#ifndef TYPES_H__C7F5152C_CDBF_47E7_8B68_AC30019A4911
#define TYPES_H__C7F5152C_CDBF_47E7_8B68_AC30019A4911
//
// $Id: types.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Interfaces that model C/C++ language types
//
#include "zdk/data_type.h"

class RTTI; // opaque, for implementation use only
struct TypeSystem;

struct TemplateTypeParam;
struct TemplateValueParam;


enum Access
{
    ACCESS_PRIVATE   = 0,
    ACCESS_PROTECTED = 1,
    ACCESS_PUBLIC    = 2,
};

enum Qualifier
{
    QUALIFIER_NONE              = 0x00,
    QUALIFIER_CONST             = 0x01,
    QUALIFIER_VOLATILE          = 0x02,
    QUALIFIER_CONST_VOLATILE    = 0x03,
};

enum CallingConvention
{
    CC_NORMAL   = 1,
    CC_PROGRAM  = 2,
    CC_NOCALL   = 3
};


DECLARE_ZDK_INTERFACE_(VoidType, DataType)
{
    DECLARE_UUID("49e2b570-0301-49e9-ae97-7277f9b7d387")

BEGIN_INTERFACE_MAP(VoidType)
    INTERFACE_ENTRY(VoidType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()
};


DECLARE_ZDK_INTERFACE_(QualifiedType, DataType)
{
    DECLARE_UUID("7893cefa-1cf0-410b-b978-e268e60b2771")

BEGIN_INTERFACE_MAP(QualifiedType)
    INTERFACE_ENTRY(QualifiedType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()

    virtual DataType* remove_qualifier() const = 0;

    virtual Qualifier qualifier() const = 0;
};


DECLARE_ZDK_INTERFACE_(ConstType, QualifiedType)
{
    DECLARE_UUID("11e148c4-d649-46cd-8eaf-0df4327a8c3f")

BEGIN_INTERFACE_MAP(ConstType)
    INTERFACE_ENTRY(ConstType)
    INTERFACE_ENTRY_INHERIT(QualifiedType)
END_INTERFACE_MAP()
};


DECLARE_ZDK_INTERFACE_(VolatileType, QualifiedType)
{
    DECLARE_UUID("0a4f5e3c-5e1d-40da-8ab8-2fe71c0cc479")

BEGIN_INTERFACE_MAP(VolatileType)
    INTERFACE_ENTRY(VolatileType)
    INTERFACE_ENTRY_INHERIT(QualifiedType)
END_INTERFACE_MAP()
};



DECLARE_ZDK_INTERFACE_(ArrayType, DataType)
{
    DECLARE_UUID("c8c1be85-d434-4656-bd32-fc59e0820232")

BEGIN_INTERFACE_MAP(ArrayType)
    INTERFACE_ENTRY(ArrayType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()

    virtual DataType* elem_type() const = 0;

    virtual size_t elem_count() const = 0;

    virtual addr_t first_elem_addr(DebugSymbol*) const = 0;
};


/**
 * Support for C99 _Complex
 */
DECLARE_ZDK_INTERFACE_(ComplexType, DataType)
{
    DECLARE_UUID("cd7b9a84-6f45-439a-9588-0ef412387963")

BEGIN_INTERFACE_MAP(ComplexType)
    INTERFACE_ENTRY(ComplexType)
    INTERFACE_ENTRY_INHERIT(DataType);
END_INTERFACE_MAP()

    virtual DataType* part_type() const = 0;
};


DECLARE_ZDK_INTERFACE_(DynamicArrayType, ArrayType)
{
    DECLARE_UUID("0ec841c3-66fa-4385-8af7-ddb1023ac49b")

BEGIN_INTERFACE_MAP(DynamicArrayType)
    INTERFACE_ENTRY(DynamicArrayType)
    INTERFACE_ENTRY_INHERIT(ArrayType)
END_INTERFACE_MAP()

    virtual size_t count(DebugSymbol*) const = 0;
};



DECLARE_ZDK_INTERFACE_(IntType, DataType)
{
    DECLARE_UUID("b1171681-c66a-4a22-8e52-95d62c14a4ce")

BEGIN_INTERFACE_MAP(IntType)
    INTERFACE_ENTRY(IntType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()

    virtual bool is_signed() const = 0;
};


DECLARE_ZDK_INTERFACE_(EnumType, DataType)
{
    DECLARE_UUID("e1748b38-6c84-492f-bd2d-df5d88ff3b88")

BEGIN_INTERFACE_MAP(EnumType)
    INTERFACE_ENTRY(EnumType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()
};


DECLARE_ZDK_INTERFACE_(FloatType, DataType)
{
    DECLARE_UUID("2b78185a-c472-4443-a2c6-5fc44ae5497c")

BEGIN_INTERFACE_MAP(FloatType)
    INTERFACE_ENTRY(FloatType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()
};


DECLARE_ZDK_INTERFACE_(PointerType, DataType)
{
    DECLARE_UUID("3104a5c1-08a4-4f63-b8f7-54ff56c54b37")

BEGIN_INTERFACE_MAP(PointerType)
    INTERFACE_ENTRY(PointerType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()

    // distinguish between "true pointers" and pointers
    // that implement the C++ reference type
    enum Kind { POINTER = '*', REFERENCE = '&' };

    virtual DataType* pointed_type() const = 0;

    virtual bool is_reference() const = 0;

    /**
     * @note hack, deal with C-style strings and wide strings
     */
    virtual bool is_cstring() const = 0;
    virtual bool is_ustring() const = 0;
};


DECLARE_ZDK_INTERFACE_(PtrToMemberType, DataType)
{
    DECLARE_UUID("86baa0e3-b565-4fb7-bf7a-e8d9e1526f0d")

BEGIN_INTERFACE_MAP(PtrToMemberType)
    INTERFACE_ENTRY(PtrToMemberType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()

    /**
     * compute offset from beginning of base type
     */
    virtual size_t offset() const = 0;

    virtual DataType* base_type() const = 0;
    virtual DataType* pointed_type() const = 0;
};


DECLARE_ZDK_INTERFACE_(FunType, DataType)
{
    DECLARE_UUID("3368b6be-fed3-4186-9972-760f2799c349")

BEGIN_INTERFACE_MAP(FunType)
    INTERFACE_ENTRY(FunType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()

    virtual DataType* return_type() const = 0;

    /**
     * @return number of parameters this function takes
     */
    virtual size_t param_count() const = 0;

    /**
     * @return the type of the n-th parameter
     * @throw out_of_range
     */
    virtual DataType* param_type(size_t) const = 0;

    /**
     * @return true if has ellipsis
     */
    virtual bool has_variable_args() const = 0;

    /**
     * The debugger tries to determine the return
     * type of a function in the debugged program by
     * using the debug symbols (currently the STABS
     * and DWARF-2 formats are supported). However, if
     * the information is not available, the function
     * will be assumed to return 'int' and take variable
     * arguments (as in good old K&R C). In this case,
     * the is_return_type_strict will return false, thus
     * indicating to the expression interpreter that it
     * is okay to silently cast the return value to other
     * types.
     */
    virtual bool is_return_type_strict() const = 0;
};


////////////////////////////////////////////////////////////////
// ClassType helpers:
//
DECLARE_ZDK_INTERFACE_(BaseClass, ZObject)
{
    DECLARE_UUID("4137efd6-a419-4329-a666-94fa2689eea2")

    /**
     * @param typeSys provides access to string cache,
     * may be NULL
     */
    virtual SharedString* name(TypeSystem* typeSys) const = 0;

    virtual DataType* type() const = 0;

    virtual off_t bit_offset() const = 0;

    virtual off_t offset() const = 0;

    /**
     * @return index if this base is virtual, otherwise zero
     */
    virtual size_t virtual_index() const = 0;
};


DECLARE_ZDK_INTERFACE_(Member, ZObject)
{
    DECLARE_UUID("d286ef5a-0881-4d90-af44-006a3a19bef5")

    virtual SharedString* name() const = 0;

    virtual DataType* type() const = 0;

    virtual bool is_static() const = 0;

    /**
     * @note for static variables
     */
    virtual SharedString* linkage_name() const = 0;

    virtual off_t bit_offset() const = 0;
};


DECLARE_ZDK_INTERFACE_(Method, ZObject)
{
    DECLARE_UUID("87cac581-5565-4aa0-a706-2986cf8a55eb")

    virtual SharedString* name() const = 0;

    virtual SharedString* linkage_name() const = 0;

    virtual FunType* type() const = 0;

    /**
     * @return const-volatile qualifier
     */
    virtual Qualifier qualifier() const = 0;

    virtual Access access() const = 0;

    virtual addr_t start_addr() const = 0;

    virtual addr_t end_addr() const = 0;

    virtual bool is_virtual() const = 0;

    virtual bool is_inline() const = 0;

    virtual bool is_static() const = 0;

    virtual CallingConvention calling_convention() const = 0;

    /**
     * @return the offset in the virtual table
     */
    virtual off_t vtable_offset() const = 0;
};


/**
 * Classes, structs, and unions
 */
DECLARE_ZDK_INTERFACE_(ClassType, DataType)
{
    DECLARE_UUID("695fc2a3-f5c6-4948-ad32-65136b864486")

BEGIN_INTERFACE_MAP(ClassType)
    INTERFACE_ENTRY(ClassType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()

    virtual SharedString* unqualified_name() const = 0;

    virtual bool has_vtable(Thread*) const = 0;

    virtual bool is_union() const = 0;

    /**
     * @return number of base classes
     */
    virtual size_t base_count() const = 0;

    /**
     * @return n-th base
     * @throw out_of_range
     */
    virtual const BaseClass* base(size_t) const = 0;

    /**
     * Lookup base class by name, if offset is not null, then
     * fill out the offset of the base class in the derived
     */
    virtual const BaseClass* lookup_base
      (
        const SharedString* name,
        off_t* offset = NULL,
        bool  recursive = false
      ) const = 0;

    /**
     * @return number of virtual bases
     */
    virtual size_t virtual_base_count() const = 0;

    /**
     * @return number of data members
     */
    virtual size_t member_count() const = 0;

    /**
     * @return n-th member
     * @throw out_of_range
     */
    virtual const Member* member(size_t n) const = 0;

    /**
     * @return number of methods (member functions)
     */
    virtual size_t method_count() const = 0;

    /**
     * @return n-th method
     * @throw out_of_range
     */
    virtual const Method* method(size_t) const = 0;

    /**
     * @note pass in the current thread, just in case the DebugInfoReader
     * needs to fetch some info from the virtual memory space
     */
    virtual RTTI* rtti(Thread*) const = 0;

    virtual size_t enum_template_type_param(
            EnumCallback<TemplateTypeParam*>*
            ) const = 0;

    virtual size_t enum_template_value_param(
            EnumCallback<TemplateValueParam*>*
            ) const = 0;
};


/**
 * Convenience function
 */
template<typename T>
inline bool has_vtable(const T* type, Thread* thread)
{
    bool result = false;

    if (const ClassType* klass = interface_cast<const ClassType*>(type))
    {
        result = klass->has_vtable(thread);
    }
    return result;
}


DECLARE_ZDK_INTERFACE_(TypeChangeObserver, Unknown2)
{
    DECLARE_UUID("d891b65b-ffcb-4ea7-aa3a-b60003d65e2e")

    virtual bool on_type_change(DebugSymbol*, DataType*) = 0;
};


/**
 * A pseudo-type, for dealing with pre-processor macros
 * @note it only works when compiling with -gdwarf-2 -g3 or
 * -gdwarf-23 flags
 */
DECLARE_ZDK_INTERFACE_(MacroType, DataType)
{
    DECLARE_UUID("357198bc-e9a2-49af-a880-9c3cffbd24a3")

BEGIN_INTERFACE_MAP(MacroType)
    INTERFACE_ENTRY(MacroType)
    INTERFACE_ENTRY_INHERIT(DataType)
END_INTERFACE_MAP()
};


#endif // TYPES_H__C7F5152C_CDBF_47E7_8B68_AC30019A4911
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
