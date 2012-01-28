#ifndef TYPES_H__72652349_DF83_41B5_92F9_76BAA39A9737
#define TYPES_H__72652349_DF83_41B5_92F9_76BAA39A9737
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
// Definitions for classes that model C/C++ language
// datatypes, both built-in and user-defined, aggregated
// types such as structs, arrays, classses etc.
//
#include <string>
#include "zdk/export.h"
#include "zdk/types.h"
#include "zdk/weak_ptr.h"
#include "typez/public/data_type_impl.h"
#include "typez/public/param_types.h"
#include "typez/public/value.h"
#include "dharma/object_manager.h" // for CountedInstance<>


class DebugSymbolImpl;
class NativeTypeSystem;


/**
 * A base type for forward declared types -- used in stabz
 */
CLASS IndirectType : public DataTypeImpl<DataType, ObservableNamedType>
{
protected:
    typedef DataTypeImpl<DataType, ObservableNamedType> BaseType;

    IndirectType(SharedString*, bitsize_t);

    virtual ~IndirectType() throw();

    bool is_equal(const DataType* type) const;

public:
    DECLARE_UUID("3cb2169a-721d-4d41-a52a-adb3c25e524b")

    BEGIN_INTERFACE_MAP(IndirectType)
        INTERFACE_ENTRY(IndirectType)
        INTERFACE_ENTRY_INHERIT(BaseType)
    END_INTERFACE_MAP()

    virtual DataType* link() const = 0;
};


/**
 * The void type.
 * "Form is empty and emptiness is form" (The Heart Sutra)
 * "Nequaquam vacuum" (Dark Ages Catholic saying)
 */
CLASS VoidTypeImpl : public DataTypeImpl<VoidType>
{
protected:
    friend class NativeTypeSystem;

    VoidTypeImpl();

    bool is_equal(const DataType* type) const;

private:
    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    bool is_fundamental() const { return true; }

    virtual size_t parse(const char* value, Unknown2*) const
    {
        return 0;
    }
};


/**
 * Represents integral types of various sizes, signed and unsigned.
 */
CLASS IntTypeImpl : public DataTypeImpl<IntType>
{
    typedef DataTypeImpl<IntType> BaseType;
    friend class MemberImpl;
    friend class NativeTypeSystem;

DECLARE_UUID("0d291f7f-56d2-4050-887e-ac693c006a15")

BEGIN_INTERFACE_MAP(IntTypeImpl)
    INTERFACE_ENTRY(IntTypeImpl)
    INTERFACE_ENTRY_INHERIT(BaseType)
END_INTERFACE_MAP()

public:
    bool is_signed() const { return isSigned_; }

protected:
    IntTypeImpl(SharedString*, bitsize_t bitSize, bool isSigned);
    IntTypeImpl(bitsize_t bitSize, bitsize_t bitOffs, bool isSigned);

    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    void write(DebugSymbol*, const Buffer*) const;

    int compare(const char*, const char*) const;

    bool is_fundamental() const { return true; }

    bool is_equal(const DataType*) const;

    size_t parse(const char*, Unknown2*) const;

    std::string description() const;

private:
    bool isSigned_ : 1;
    bool isChar_ : 1;

    bitsize_t bitOffs_ : (__WORDSIZE - 2);
};


/**
 * Represents floating-point numbers of various precisions.
 */
CLASS FloatTypeImpl : public DataTypeImpl<FloatType>
{
    DECLARE_UUID("f13e2bd3-191a-4d7e-b469-2b6221ccf9be")

    typedef DataTypeImpl<FloatType> BaseType;

protected:
    friend class NativeTypeSystem;

    FloatTypeImpl(SharedString*, size_t bitSize);

public:
    int compare(const char*, const char*) const;

private:
    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    void write(DebugSymbol*, const Buffer*) const;

    bool is_fundamental() const { return true; }

    bool is_equal(const DataType*) const;

    size_t parse(const char*, Unknown2*) const;
};




/**
 * This class has no corresponding type in the
 * C/C++ languages; rather, it uses the "decorator"
 * pattern to wrap around another type; its children
 * add constness and/or volatile qualifiers to a type
 * or turn a base type into a pointer or reference type.
 * @note The same technique is used in the dwarfz
 * library (the classes have similar names, within
 * the Dwarf namespace). There is a bit of duplication
 * that stems from trying to keep the dwarfz lib as a
 * standalone C++ wrapper to libdwarf.
 */
template<typename T = DataType>
CLASS DecoratorType : public DataTypeImpl<T, ObservableNamedType>
{
    typedef DataTypeImpl<T, ObservableNamedType> Base;

public:
    DECLARE_UUID("c5655991-5826-4cec-b6e2-3e45b6c99c3a")

    BEGIN_INTERFACE_MAP(DecoratorType)
        INTERFACE_ENTRY(DecoratorType)
        INTERFACE_ENTRY_DELEGATE(observ_)
        INTERFACE_ENTRY_INHERIT(Base)
    END_INTERFACE_MAP()

    ////////////////////////////////////////////////////////////
    virtual void update_name() {}

    ////////////////////////////////////////////////////////////
    void on_state_change(Subject* subject)
    {
        assert(subject);

        update_name();
    }

protected:
    virtual ~DecoratorType() throw() {}

    ////////////////////////////////////////////////////////////
    DecoratorType(SharedString* name, DataType& type, size_t nbits)
        : Base(name, nbits)
        , type_(&type)
        , observ_(create_observer_delegate(this))
    {
        assert(type.ref_count() > 0);
        type.attach_to_observer(observ_.get());
    }

    ////////////////////////////////////////////////////////////
    /// @return the decorated type
    DataType& type() const
    {
        assert(type_.get());
        return *type_;
    }

    ////////////////////////////////////////////////////////////
    void write(DebugSymbol* sym, const Buffer* buf) const
    {
        // delegate to decorated type
        type().write(sym, buf);
    }

    ////////////////////////////////////////////////////////////
    SharedString* read(DebugSymbol* sym, DebugSymbolEvents* e) const
    {
        // delegate to decorated type
        return type().read(sym, e);
    }

    ////////////////////////////////////////////////////////////
    int compare(const char* lhs, const char* rhs) const
    {
        return type().compare(lhs, rhs);
    }

    ////////////////////////////////////////////////////////////
    bool is_fundamental() const
    {
        return type_ ? type_->is_fundamental() : false;
    }

    ////////////////////////////////////////////////////////////
    bool is_equal(const DataType* type) const
    {
        if (const DecoratorType* that =
                interface_cast<const DecoratorType*>(type))
        {
            if (!interface_cast<const T*>(type))
            {
                return false;
            }
            if (!type_)
            {
                return !that->type_;
            }
            else if (that->type_)
            {
                return type_->is_equal(that->type_.get());
            }
        }
        else if (type_)
        {
            return type_->is_equal(type);
        }
        return false;
    }

    ////////////////////////////////////////////////////////////
    size_t parse(const char* value, Unknown2* unk) const
    {
        return type_ ? type_->parse(value, unk) : 0;
    }

private:
    RefPtr<DataType> type_; // the "decorated" type
    RefPtr<Observer> observ_;
};


/**
 * Represents and reads pointers and references, assuming
 * that the C++ compiler, under the hood, implements refs
 * as pointers.
 */
CLASS PointerTypeImpl : public DecoratorType<PointerType>
{
    friend class NativeTypeSystem;
    typedef DecoratorType<PointerType> BaseType;

public:

BEGIN_INTERFACE_MAP(PointerTypeImpl)
    INTERFACE_ENTRY_INHERIT(BaseType)
END_INTERFACE_MAP()

    ~PointerTypeImpl() throw();

protected:
    /**
     * Construct a pointer type that refers variables of
     * the `type' type; refType tells if it implements
     * e "real" pointer or a C++ reference.
     */
    PointerTypeImpl(TypeSystem&, Kind, DataType& type);

    void update_name();

    SharedString* make_pointer_name(const char*, ENFORCE_REF_PTR_) const;

public:
    bool is_cstring() const;

    bool is_ustring() const;

    bool is_reference() const { return pointerOrReference_ == REFERENCE; }

    int compare(const char*, const char*) const;

    size_t parse(const char*, Unknown2*) const;

    DataType* pointed_type() const { return &type(); }

private:
    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    void write(DebugSymbol*, const Buffer*) const;

    bool is_fundamental() const { return true; }

    bool is_equal(const DataType*) const;

private:
    Kind pointerOrReference_;
};


CLASS PtrToMemberTypeImpl : public DataTypeImpl<PtrToMemberType>
{
    WeakPtr<TypeSystem> types_;
    RefPtr<DataType>    base_;
    RefPtr<DataType>    type_;

    RefPtr<SharedString> make_name(TypeSystem&, const DataType&, const DataType&);

    bool is_fundamental() const { return true; }

    size_t parse(const char*, Unknown2*) const { return 0; }

    bool is_equal(const DataType*) const;

public:
    ~PtrToMemberTypeImpl() throw() { }
    PtrToMemberTypeImpl(TypeSystem&, DataType& base, DataType& type);

BEGIN_INTERFACE_MAP(PtrToMemberTypeImpl)
    INTERFACE_ENTRY_INHERIT(DataTypeImpl<PtrToMemberType>)
END_INTERFACE_MAP()

    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    DataType* base_type() const { return base_.get(); }
    DataType* pointed_type() const { return type_.get(); }
    size_t offset() const { return 0; }
};


/**
 * A pseudo-type for handling macro-definitions.
 */
CLASS MacroTypeImpl : public DataTypeImpl<MacroType>
{
public:
    ~MacroTypeImpl() throw() { }

BEGIN_INTERFACE_MAP(MacroTypeImpl)
    INTERFACE_ENTRY_INHERIT(DataTypeImpl<MacroType>)
END_INTERFACE_MAP()

protected:
    friend class NativeTypeSystem;

    MacroTypeImpl(TypeSystem&);

    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    void write(DebugSymbol*, const Buffer*) const;

    size_t parse(const char*, Unknown2*) const;

    bool is_fundamental() const { return true; }

    bool is_equal(const DataType* other) const;

private:
    static SharedString* type_name();

    //use pointer type impl just
    // for the convenience of dealing with C-style strings
    RefPtr<PointerType> impl_;
};


/**
 * const-qualified type
 */
CLASS ConstTypeImpl : public DecoratorType<ConstType>
{
public:
    BEGIN_INTERFACE_MAP(ConstTypeImpl)
        INTERFACE_ENTRY_INHERIT(DecoratorType<ConstType>)
        INTERFACE_ENTRY_DELEGATE(&type())
    END_INTERFACE_MAP()

    ~ConstTypeImpl() throw() {}

protected:
    friend class NativeTypeSystem;

    explicit ConstTypeImpl(DataType&);

    void update_name();

    DataType* remove_qualifier() const { return &type(); }

    Qualifier qualifier() const { return QUALIFIER_CONST; }
};


/**
 * volatile-qualified type
 */
CLASS VolatileTypeImpl : public DecoratorType<VolatileType>
{
public:
    BEGIN_INTERFACE_MAP(VolatileTypeImpl)
        INTERFACE_ENTRY_INHERIT(DecoratorType<VolatileType>)
        INTERFACE_ENTRY_DELEGATE(&type())
    END_INTERFACE_MAP()

    ~VolatileTypeImpl() throw() {}

protected:
    friend class NativeTypeSystem;

    explicit VolatileTypeImpl(DataType&);

    void update_name();

    DataType* remove_qualifier() const { return &type(); }

    Qualifier qualifier() const { return QUALIFIER_VOLATILE; }
};


CLASS TypeAlias : public DecoratorType<>
{
public:
    BEGIN_INTERFACE_MAP(TypeAlias)
        INTERFACE_ENTRY_INHERIT(DecoratorType<>)
        INTERFACE_ENTRY_DELEGATE(&type())
    END_INTERFACE_MAP()

    ~TypeAlias() throw() {}

protected:
    friend class NativeTypeSystem;

    TypeAlias(SharedString* name, DataType& type)
        : DecoratorType<>(name, type, type.bit_size())
    { }
};


class ClassTypeImpl;
class MemberImpl;
class MethodImpl;
class RTTI;


CLASS BaseImpl : public ZObjectImpl<BaseClass>
{
public:
    BaseImpl
      (
        Access access,
        off_t  bitOffs,
        size_t bitSize,
        DataType&,
        size_t vindex
      );

    SharedString* name(TypeSystem*) const;

    DataType* type() const;

    size_t virtual_index() const { return vindex_; }

    off_t bit_offset() const; // offset in bits

    off_t offset() const; // offset in bytes

    /**
     * implements DataType behavior
     */
    virtual void read(DebugSymbol&, addr_t, DebugSymbolEvents*);

private:
    RefPtr<MemberImpl> impl_;
    unsigned short vindex_;
    const Access access_;
};


/**
 * Implements the MemberData interface;
 * models a class, struct or union data member.
 */
CLASS MemberImpl : public ZObjectImpl<Member>
{
public:
    ~MemberImpl() throw() {}

    MemberImpl
      (
        TypeSystem*     types,
        SharedString*   name,
        off_t           bitOffs,
        size_t          bitSize,
        DataType&       type,
        bool            isStatic,
        DebugSymbol*    value = NULL
      );

    SharedString* name() const;

    off_t bit_offset() const { return bitOffs_; }

    size_t bit_size() const { return bitSize_; }

    DataType* type() const;

    bool is_static() const { return isStatic_; }

    void set_linkage_name(const RefPtr<SharedString>& name)
    { linkageName_ = name; }

    SharedString* linkage_name() const { return linkageName_.get(); }

    virtual void read
      (
        DebugSymbol& parent,
        addr_t addr,
        DebugSymbolEvents*
      );

    void read
      (
        DebugSymbol& parent,
        addr_t addr,
        DebugSymbolEvents*,
        unsigned long mask
      );

    void set_name(const RefPtr<SharedString>& name) { name_ = name; }

private:
    /**
     * Helper called from the read() method
     */
    void add_child_symbol(DebugSymbol& parent, addr_t, DebugSymbolEvents*);

    RefPtr<SharedString>    name_;
    bool                    isStatic_ : 4;
    bool                    isBitField_ : 4;
    const off_t             bitOffs_;
    size_t                  bitSize_;
    RefPtr<DataType>        type_;
    RefPtr<SharedString>    linkageName_;
    RefPtr<DebugSymbol>     value_; // for constants
};



/**
 * A member function
 */
CLASS MethodImpl : public ZObjectImpl<Method>
{
public:
    BEGIN_INTERFACE_MAP(MethodImpl)
        INTERFACE_ENTRY(Method)
    END_INTERFACE_MAP()

    MethodImpl
      (
        SharedString* name,
        const RefPtr<SharedString>& linkageName, // mangled
        FunType* functionType,
        Access access,
        bool isVirtual,
        Qualifier qualifier
      );

    ~MethodImpl() throw() {}

    SharedString* name() const { return name_.get(); }

    SharedString* linkage_name() const { return linkageName_.get(); }

    FunType* type() const { return get_pointer(funType_); }

    void set_type(const RefPtr<FunType>& funType)
    {
        assert(funType->ref_count() > 1);
        funType_ = funType;
    }

    Access access() const { return access_; }

    Qualifier qualifier() const { return qualifier_; }

    addr_t start_addr() const;

    void set_start_addr(addr_t addr)
    {
        assert(addr_ == 0);
        addr_ = addr;
    }
    addr_t end_addr() const;

    off_t vtable_offset() const;

    void set_vtable_offset(off_t offset)
    {
        vtableOffset_ = offset;
    }

    bool is_virtual() const { return isVirtual_; }

    bool is_inline() const { return isInline_; }

    bool is_static() const;

    void set_inline(bool flag) { isInline_ = flag; }

    CallingConvention calling_convention() const
    {
        return callingConvention_;
    }

    void set_calling_convention(CallingConvention);

private:
    RefPtr<SharedString>    name_;
    RefPtr<SharedString>    linkageName_;
    WeakPtr<FunType>        funType_;
    const Access            access_ : 3;
    const bool              isVirtual_ : 1;
    bool                    isInline_ : 1;
    const Qualifier         qualifier_ : 3;
    CallingConvention       callingConvention_ : 3;
    addr_t                  addr_;
    off_t                   vtableOffset_;
};


/**
 * Represents aggregate user types such as
 * structs, unions, C++ classes
 */
CLASS ClassTypeImpl
    : public DataTypeImpl<ClassType, ObservableNamedType>
{
    typedef DataTypeImpl<ClassType, ObservableNamedType> BaseType;

public:
    DECLARE_UUID("38ecb509-72e7-4f50-80ab-801649d670bd")

BEGIN_INTERFACE_MAP(ClassTypeImpl)
    INTERFACE_ENTRY(ClassTypeImpl)
    INTERFACE_ENTRY_INHERIT(BaseType)
END_INTERFACE_MAP()

    typedef RefPtr<MemberImpl> MemberPtr;
    typedef std::vector<MemberPtr> MemberList;

    typedef std::vector<RefPtr<MethodImpl> > MethodList;
    typedef std::auto_ptr<MethodList> MethodListPtr;
    typedef std::vector<RefPtr<BaseImpl> > BaseList;
    typedef std::auto_ptr<BaseList> BaseListPtr;

    typedef std::vector<RefPtr<TemplateTypeParam> > TemplateTypeList;
    typedef std::auto_ptr<TemplateTypeList> TemplateTypeListPtr;

    typedef std::vector<RefPtr<TemplateValueParam> > TemplateValueList;
    typedef std::auto_ptr<TemplateValueList> TemplateValueListPtr;

    ClassTypeImpl
      (
        TypeSystem*,
        SharedString*,
        size_t,
        bool isUnion = false
      );

    ~ClassTypeImpl() throw();

    static off_t
        offset_to_top(Thread&, const BaseClass&, addr_t, off_t);

    size_t parse(const char*, Unknown2*) const;

    MemberImpl& add_member
      (
        const RefPtr<SharedString>& name,
        RefPtr<SharedString> linkageName, // may be NULL
        off_t bitOffset,
        size_t bitSize,
        DataType&,
        bool isStatic = false,
        DebugSymbol* value = NULL
      );

    void add_base
      (
        DataType&,
        off_t bitOffset, // can be negative
        Access,
        bool isVirtual,
        ClassType* owner = NULL
      );

    RefPtr<MethodImpl> add_method
      (
        RefPtr<SharedString> name,
        RefPtr<SharedString> linkageName,
        FunType*,
        Access = ACCESS_PUBLIC,
        bool isVirtual = false,
        Qualifier = QUALIFIER_NONE
      );

    void add_template_type_param(TemplateTypeParam&);
    void add_template_value_param(TemplateValueParam&);

    const MemberList& member_data() const { return members_; }

    const BaseClass* lookup_base
      (
        const SharedString* name,
        off_t* offset = NULL,
        bool recursive = false
      ) const;

    RTTI* rtti(Thread*) const;

    std::string description() const;

    bool is_union() const { return isUnion_; }

    ///// ClassType interface /////
    SharedString* unqualified_name() const;

    size_t base_count() const
    {
        return bases_.get() ? bases_->size() : 0;
    }

    const BaseClass* base(size_t) const;

    bool has_vtable(Thread*) const;

    size_t virtual_base_count() const { return virtualBasesCount_; }

    size_t member_count() const { return members_.size(); }

    /**
     * @note co-variant type returned
     */
    const MemberImpl* member(size_t) const;

    size_t method_count() const;

    /**
     * @note co-variant type returned
     */
    const MethodImpl* method(size_t) const;

protected:
    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    SharedString* read_impl(DebugSymbol&,
                            addr_t addr,
                            DebugSymbolEvents*,
                            bool checkRTTI) const;

    /**
     * Attempt to discover the dynamic (run-time) type, this
     * class may be a (virtual) base of a derived object; if
     * found, read the object using the run-time type rather
     * than this ClassTypeImpl
     */
    SharedString* check_rtti(DebugSymbol&, DebugSymbolEvents*) const;

    bool is_fundamental() const { return false; }

    bool is_equal(const DataType* type) const;

    bool inherit_rtti(BaseClass&, Thread*) const;

    /**
     * When namespace support is not preset, the prefix(es) can
     * be inferred from member names (For e.g. A::Foo::bah() -->
     * the class name is A::Foo).
     */
    void compute_full_name(const RefPtr<SharedString>& linkageName);

    size_t
    enum_template_type_param(EnumCallback<TemplateTypeParam*>*) const;

    size_t
    enum_template_value_param(EnumCallback<TemplateValueParam*>*) const;

private:
    WeakPtr<TypeSystem>     typeSystem_;
    RefPtr<SharedString>    unqualifiedName_;
    BaseListPtr             bases_;
    MemberList              members_;
    MethodListPtr           methods_;

    TemplateTypeListPtr     templateTypeParams_;

    mutable RefPtr<RTTI>    rtti_;
    mutable bool            rttiComputed_ : 1;
    bool                    isUnion_ : 1;
    unsigned short          virtualBasesCount_;
};


/**
 * Run-time-type-info: encapsulates some voodoo that
 * helps determining the actual type at runtime, in
 * case of C++ polymorphism.
 */
CLASS RTTI : public RefCountedImpl<>
{
public:
    explicit RTTI(const RefPtr<MemberImpl>&);

    RTTI(const RTTI& other)
        : processing_(false)
        , typename_(other.typename_)
        , bitOffset_(other.bitOffset_)
        , offset_(other.offset_)
    {}

    virtual ~RTTI() throw();

    static RefPtr<SharedString> parse_for_type(SharedString*);

    RefPtr<SharedString> type_name(DebugSymbol&);

    void add_vptr_bit_offset(off_t);

    off_t vptr_bit_offset() const { return bitOffset_; }

    off_t vptr_offset() const { return offset_; }

    bool processing_;

private:
    RefPtr<SharedString>    typename_;
    off_t                   bitOffset_;
    mutable off_t           offset_;
};


/**
 * C/C++ array types
 *
 * @note assume that the index is of an integer type, otherwise
 * we would be dealing with an AssociativeContainerType
 */
CLASS ArrayTypeImpl : public DataTypeImpl<ArrayType>
{
    friend class NativeTypeSystem;
    typedef DataTypeImpl<ArrayType> BaseType;

public:
    typedef std::pair<int64_t, int64_t> Range;

protected:
    ArrayTypeImpl(TypeSystem&, DataType& elemType, const Range&);

    void read_elements( DebugSymbolImpl&,
                        addr_t,
                        DebugSymbolEvents*) const;

    RefPtr<DataType> elem_type_ref() const;

public:
    ~ArrayTypeImpl() throw() { }

    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    bool is_fundamental() const { return false; }

    bool is_equal(const DataType* type) const;

    size_t parse(const char*, Unknown2*) const;

    DataType* elem_type() const;

    size_t elem_count() const;

    addr_t first_elem_addr(DebugSymbol* sym) const
    {
        return sym ? sym->addr() : 0;
    }

private:
    // Helper functions called from ctor's initializer list.
    static SharedString* get_name(TypeSystem&,
                                  DataType& elemType,
                                  const Range&
                                 );
    static size_t get_bit_size(DataType& elemType, const Range&);

    WeakDataTypePtr elemType_;

    Range range_;
    const int wordSize_;
};



/**
 * Models function types (for both standalone and member functions).
 */
CLASS FunTypeImpl
    : public DataTypeImpl<FunType, ObservableNamedType>
#ifdef DEBUG_OBJECT_LEAKS
    , public CountedInstance<FunTypeImpl>
#endif
{
    DECLARE_UUID("8e719457-e16d-44a7-90ad-a6c8f4568b6b")

    typedef DataTypeImpl<FunType, ObservableNamedType> BaseType;

protected:
    friend class NativeTypeSystem;

    explicit FunTypeImpl(
        TypeSystem&         types,
        RefPtr<DataType>    retType,
        const ParamTypes*   paramTypes = 0);

    explicit FunTypeImpl(
        TypeSystem&         types,
        DataType&           retType,
        DataType* const*    argTypes,
        size_t              argCount);

public:
    explicit FunTypeImpl(
        TypeSystem&         types,
        SharedString&       name,
        WeakDataTypePtr     retType = WeakDataTypePtr(), // void
        const ParamTypes*   paramTypes = 0);

    ~FunTypeImpl() throw();

    void on_state_change(Subject*);

    SharedString* make_pointer_name(const char*, ENFORCE_REF_PTR_) const;

    static RefPtr<SharedString> make_pointer_name(
        const char*,            // name
        const WeakDataTypePtr&, // ret type
        const ParamTypes*);

    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    int compare(const char*, const char*) const;

    bool is_fundamental() const { return false; }

    bool is_equal(const DataType* type) const;

    DataType* return_type() const;

    size_t param_count() const { return paramTypes_.size(); }

    DataType* param_type(size_t n) const;

    bool has_variable_args() const { return vargs_; }

    void set_variable_args() { vargs_ = true; }

    bool is_return_type_strict() const { return strict_; }

    void set_return_type_strict(bool strict) { strict_ = strict; }

    // todo: is there any case where I would want to read
    // in a function pointer?
    virtual size_t parse(const char* value, Unknown2*) const
    {
        assert(value);
        return 0;
    }

    virtual std::string description() const { return "Function"; }

private:
    // helper, called by ctors
    void init(TypeSystem&, const ParamTypes*);

    WeakDataTypePtr retType_;
    ParamTypes paramTypes_;

    bool vargs_  : 4;
    bool strict_ : 4;

    RefPtr<Observer> observ_;
};


/**
 * @return the maximum number of array elements to be displayed
 */
size_t max_array_range();

#endif // TYPES_H__72652349_DF83_41B5_92F9_76BAA39A9737
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
