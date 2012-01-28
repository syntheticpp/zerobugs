#ifndef TYPE_SYSTEM_IMPL_H__5E11E6D1_243C_4A9A_844B_C73987076345
#define TYPE_SYSTEM_IMPL_H__5E11E6D1_243C_4A9A_844B_C73987076345
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
#include <boost/limits.hpp>
#include <map>
#include "zdk/config.h"
#include "zdk/data_type.h"
#include "zdk/type_system.h"
#include "zdk/ref_ptr.h"
#include "zdk/weak_ptr.h"
#include "zdk/weak_ref_impl.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "funkey.h"

class FunTypeImpl;
class PointerTypeImpl;


/**
 * The debugger needs to know about data types in order to represent
 * program variables correctly.
 *
 * This class implements support for C/C++ data types, assuming that
 * we are debugging native programs (as opposed to cross-debugging).
 */
class ZDK_LOCAL NativeTypeSystem : public ZObjectImpl<TypeSystem>
{
public:
    static const bitsize_t max_int_bits =
        std::numeric_limits<unsigned long long>::digits;

    static const size_t max_float_bytes = MAX_FLOAT_BYTES;

BEGIN_INTERFACE_MAP(NativeTypeSystem)
    INTERFACE_ENTRY(TypeSystem)
    INTERFACE_ENTRY_DELEGATE(stringCache_)
END_INTERFACE_MAP()

    explicit NativeTypeSystem(const size_t& wordSize);

	virtual ~NativeTypeSystem() throw();

    virtual DataType* get_void_type();

    /**
     * @return a pointer to an object that represents an
     * integer type, of the specified size (in bits);
     * @note the size is specified in bits, so that we can
     * model bit-fields in C/C++ structs and classes.
     * @param name specifies the name of the type
     * @note this parameter is used only when the type is
     * first created; if a type of the given bit width and
     * signed-ness exists, then that type object is returned.
     * @param bitSize the size of the type, in bits, including
     * the sign bit
     * @param isSigned if true, the returned object represents
     * a signed integer type.
     * Example:
     * @code
     *  get_int_type(new SharedStringImpl("long"), 32, true);
     * @endcode
     */
    virtual DataType* get_int_type(SharedString*   name,
                                   bitsize_t       bitSize,
                                   bool            isSigned) ;

    virtual DataType* get_bool_type(size_t nbits);

    virtual DataType* get_array_type(int64_t rangeLow,
                                     int64_t rangeHigh,
                                     DataType* elemType);

    virtual ClassType* get_class_type(const char* name,
                                     size_t bitSize,
                                     bool isUnion = false);

    virtual ClassType* get_unnamed_class_type(size_t bitSize,
                                     bool isUnion = false);

    /**
     * @return a pointer to an object that represents a floating
     * point type, of the specified width.
     * @note the size is given in bytes, since, unlike for ints,
     * bit-fields of floating type are not allowed.
     */
    virtual DataType* get_float_type(SharedString*   name,
                                     size_t          byteSize);

    /**
     * Given a data type, return the object that represents a pointer
     * to that type. For example, if pointedType represents a 32-bit,
     * signed integer, the returned object represents type int* type.
     */
    virtual PointerType* get_pointer_type(DataType* pointedType);

    /**
     * Given a data type, return the object that represents a reference
     * to that type. For example, if pointedType represents a 32-bit,
     * signed integer, the returned object represents type int& type.
     */
    virtual DataType* get_reference_type(DataType* referencedType);

    /**
     * Given a data type, return an object representing the qualified
     * type. For example, if the data type is int and the qualifier is
     * QUALIFIER_CONST, the returned object will represent the
     * "const int" type.
     */
    virtual DataType* get_qualified_type(DataType*, Qualifier);

    /**
     * @return an object that represents a function type.
     * @param retType pointer to the object representing the return type
     * @todo: may NULL be passed in for function returning void?
     * @param argTypes pointer to an array of types; the argument types;
     *  must contain argCount elements. May be NULL if argCount is zero.
     * @param argCount the number of arguments that the function takes.
     */
    virtual FunType* get_fun_type( DataType* retType,
                                   DataType* const*  argTypes,
                                   size_t argCount,
                                   bool varArgs,
                                   bool strictReturnType = true );

    /**
     * Return a pseudo-type so that we can evaluate macros as
     * constant strings (what the macro expands to).
     */
    virtual DataType* get_macro_type();

    virtual DataType* get_string_type();
    virtual DataType* get_wide_string_type();

    bool use_auto_rtti() const { return rttiAuto_; }

    void set_auto_rtti(bool rttiAuto) { rttiAuto_ = rttiAuto; }

    DataType* manage(DataType* type)
    { assert(type); types_.push_back(type); return type; }

    size_t size() const { return types_.size(); }

    size_t word_size() const { return wordSize_; }

    DebugSymbol* create_debug_symbol(
        DebugInfoReader*,
        Thread*,
        DataType*,
        SharedString*,
        addr_t,
        SharedString* declFile = NULL,
        size_t declLine = 0,
        bool isReturnValue = false) const;

    //<StringCache>
    SharedString* get_string(const char* s, size_t len = 0)
    { return stringCache_->get_string(s, len); }

    SharedString* get_string(SharedString* str)
    { return stringCache_->get_string(str); }
    //<StringCache>

private:
    PointerType* get_pointer_type(PointerType::Kind, DataType*);

    DataType* get_complex_float(SharedString*, size_t);
    DataType* get_complex_int(SharedString*, size_t);
    DataType* get_ptr_to_member_type(DataType*, DataType*);

private:
    typedef RefPtr<DataType> DataTypePtr;
    typedef ext::hash_map<RefPtr<SharedString>, RefPtr<PointerType> > PtrMap;
    typedef ext::hash_map<RefPtr<SharedString>, DataTypePtr> AliasMap;
    typedef std::map<FunTypeKey, RefPtr<FunTypeImpl> > FunMap;

    typedef std::map<size_t, DataTypePtr> BoolMap;
    typedef std::vector<DataTypePtr> TypeTable;

    Mutex               mutex_;
    RefPtr<StringCache> stringCache_;
    RefPtr<DataType>    voidType_;
    RefPtr<DataType>    intTypes_[2][max_int_bits];
    RefPtr<DataType>    floatTypes_[max_float_bytes];
    const size_t&       wordSize_; // reference to wordSize inside the target
    BoolMap             boolTypes_;
    AliasMap            aliases_;
    TypeTable           types_;
    PtrMap              ptrMap_;   // pointer and reference types, by name
    FunMap              funMap_;   // function types, indexed by name
    RefPtr<DataType>    macro_;
    bool                rttiAuto_;
};

#endif // TYPE_SYSTEM_IMPL_H__5E11E6D1_243C_4A9A_844B_C73987076345
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
