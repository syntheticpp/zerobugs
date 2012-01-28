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
#include <iostream>
#include <sstream>
#include "public/complex.h"
#include "public/enum_type.h"
#include "public/types.h"
#include "public/native_type_system.h"
#include "private/int_types.h"
#include "generic/lock.h"
#include "dharma/environ.h"
#include "zdk/check_ptr.h"
#include "zdk/type_system_util.h"
#include "public/debug_symbol_impl.h"
#include "public/string_pool.h"


#if 1
 #define THREAD_SAFE Lock<Mutex> lock(mutex_)
#else
 #define THREAD_SAFE// as nothing: no locking
#endif


using namespace std;


NativeTypeSystem::NativeTypeSystem(const size_t& wordSize)
    : stringCache_(new StringPool)
    , wordSize_(wordSize)
    , rttiAuto_(env::get_bool("ZERO_AUTO_RTTI", true))
{
}



NativeTypeSystem::~NativeTypeSystem() throw()
{
#if 0 //DEBUG
    clog << __PRETTY_FUNCTION__ << endl;
#endif
}



DataType* NativeTypeSystem::get_void_type()
{
    THREAD_SAFE;
    if (!voidType_)
    {
        voidType_ = new VoidTypeImpl;
    }
    return voidType_.get();
}


DataType* NativeTypeSystem::get_int_type
(
    SharedString*   name,
    bitsize_t       nbits,
    bool            isSigned
)
{
    assert(nbits);

    if ((nbits == 0) || (nbits > max_int_bits))
    {
        cerr << "*** Warning: int size not supported: ";
        cerr << name << " is " << nbits << " bits wide\n";
        return 0;
    }
    THREAD_SAFE;

    RefPtr<DataType>& type = intTypes_[isSigned][nbits - 1];
    if (type == 0)
    {
        RefPtr<SharedString> typeName (name);

        if (!name)
        {
            ostringstream os;
            if (!isSigned) { os << 'u'; }
            os << "int";
            if (nbits != 32)
            {
                os << nbits << "_t";
            }
            typeName = shared_string(os.str());
        }
        typeName = stringCache_->get_string(typeName.get());
        type = new IntTypeImpl(typeName.get(), nbits, isSigned);
    }
    else if (name && !type->name()->is_equal2(name))
    {
        AliasMap::iterator i = aliases_.find(name);
        if (i == aliases_.end())
        {
            type = new TypeAlias(name, *type);

            i = aliases_.insert(make_pair(name, type)).first;
        }
        type = i->second;
    }
    return type.get();
}


DataType*
NativeTypeSystem::get_float_type(SharedString* name, size_t nbytes)
{
    assert(nbytes);

    if ((nbytes == 0) || (nbytes > max_float_bytes))
    {
        cerr << "*** Warning: float size not supported: ";
        cerr << name << " is " << nbytes << " bytes wide\n";
        return 0;
    }
    THREAD_SAFE;

    RefPtr<DataType>& type = floatTypes_[nbytes - 1];
    if (type == 0)
    {
        const bitsize_t nbits = nbytes * Platform::byte_size;
        RefPtr<SharedString> typeName(name);
        if (!name)
        {
            ostringstream os;
            os << "__float" << nbits;
            typeName = shared_string(os.str());
        }
        type = new FloatTypeImpl(typeName.get(), nbits);
    }
    return type.get();
}


/**
 * Given a data type, return the object that represents a pointer
 * to that type. For example, if pointedType represents a 32-bit,
 * signed integer, the returned object represents type int* type.
 */
PointerType* NativeTypeSystem::get_pointer_type(DataType* pointedType)
{
    return get_pointer_type(PointerTypeImpl::POINTER, pointedType);
}


/**
 * Given a data type, return the object that represents a reference
 * to that type. For example, if type represents a 32-bit,
 * signed integer, the returned object represents type int& type.
 */
DataType* NativeTypeSystem::get_reference_type(DataType* type)
{
    return get_pointer_type(PointerTypeImpl::REFERENCE, type);
}


PointerType*
NativeTypeSystem::get_pointer_type(PointerType::Kind k, DataType* type)
{
    assert(type);
    if (!type)
    {
        return 0;
    }

    RefPtr<PointerType> ptrType(new PointerTypeImpl(*this, k, *type));
    manage(ptrType.get());
    assert(ptrType->pointed_type() == type);

    THREAD_SAFE;
    PtrMap::iterator i =
        ptrMap_.insert(make_pair(ptrType->name(), ptrType)).first;

    ptrType = i->second;
    assert(ptrType->ref_count() > 1);
    return ptrType.get();
}


/**
 * make a CV-qualified type
 */
DataType* NativeTypeSystem::get_qualified_type(DataType* t, Qualifier q)
{
    DataType* type = t;
    if (q & QUALIFIER_CONST)
    {
        type = manage(new ConstTypeImpl(*type));
    }
    if (q & QUALIFIER_VOLATILE)
    {
        type = manage(new VolatileTypeImpl(*type));
    }
    return type;
}


/**
 * @return an object that represents a function type.
 * @param retType pointer to the object representing the return type
 * @param argTypes pointer to an array of types; the argument types;
 *  must contain argCount elements. May be NULL if argCount is zero.
 * @param argCount the number of arguments that the function takes.
 */
FunType*
NativeTypeSystem::get_fun_type(DataType* retType,
                             DataType* const* argTypes,
                             size_t argCount,
                             bool varArgs,
                             bool strict)
{
    if (!retType)
    {
        retType = get_void_type();
    }
    FunTypeKey key(*retType, argTypes, argCount, varArgs, strict);

    THREAD_SAFE;

    FunMap::iterator i = funMap_.find(key);
    if (i == funMap_.end())
    {
        RefPtr<FunTypeImpl> type =
            new FunTypeImpl(*this, *retType, argTypes, argCount);

        i = funMap_.insert(make_pair(key, type)).first;

        if (varArgs)
        {
            type->set_variable_args();
        }
        type->set_return_type_strict(strict);
    }
    return i->second.get();
}


/**
 * Return a pseudo-type so that we can evaluate macros as
 * constant strings (what the macro expands to).
 */
DataType* NativeTypeSystem::get_macro_type()
{
    THREAD_SAFE;

    if (!macro_)
    {
        macro_ = new MacroTypeImpl(*this);
    }
    return macro_.get();
}


DataType* NativeTypeSystem::get_string_type()
{
    RefPtr<DataType> type = ::get_int_type(*this, (char*)0, "char");
    return get_pointer_type(type.get());
}


DataType* NativeTypeSystem::get_wide_string_type()
{
    RefPtr<DataType> type = ::get_int_type(*this, (wchar_t*)0, "wchar_t");
    return get_pointer_type(type.get());
}



////////////////////////////////////////////////////////////////
static RefPtr<SharedString> bool_true()
{
    static const RefPtr<SharedString> sTrue(shared_string("true"));
    return sTrue;
}

////////////////////////////////////////////////////////////////
static RefPtr<SharedString> bool_false()
{
    static const RefPtr<SharedString> sFalse(shared_string("false"));
    return sFalse;
}


////////////////////////////////////////////////////////////////
template<size_t N>
static RefPtr<DataType> make_bool_type()
{
    typedef typename detail::int_type<N>::type T;
    map<T, RefPtr<SharedString> > boolVal;

    boolVal[0] = bool_false();
    boolVal[1] = bool_true();

    static RefPtr<SharedString> name(shared_string("bool"));
    return new EnumTypeImpl<T>(name.get(), boolVal, true);
}


////////////////////////////////////////////////////////////////
DataType* NativeTypeSystem::get_bool_type(size_t nbits)
{
    THREAD_SAFE;
    BoolMap::iterator i = boolTypes_.find(nbits);
    if (i == boolTypes_.end())
    {
        RefPtr<DataType> type;
        if (nbits <= 8)
        {
            type = make_bool_type<8>();
        }
        else if (nbits <= 32)
        {
            type = make_bool_type<32>();
        }
        else
        {
            type = make_bool_type<64>();
        }

        i = boolTypes_.insert(i, make_pair(nbits, type.get()));
    }
    return i->second.get();
}


////////////////////////////////////////////////////////////////
DataType* NativeTypeSystem::get_array_type
(
    int64_t rangeLow,
    int64_t rangeHigh,
    DataType* elemType
)
{
    ArrayTypeImpl::Range range(rangeLow, rangeHigh);
    return manage(new ArrayTypeImpl(*this, *CHKPTR(elemType), range));
}


////////////////////////////////////////////////////////////////
ClassType* NativeTypeSystem::get_class_type
(
    const char* name,
    size_t bitSize,
    bool isUnion
)
{
    ClassType* type = NULL;
    if (name)
    {
        RefPtr<SharedString> className(get_string(name));
        type = new ClassTypeImpl(this, className.get(), bitSize, isUnion);
        manage(type);
    }
    else
    {
        type = get_unnamed_class_type(bitSize, isUnion);
    }
    return type;
}


////////////////////////////////////////////////////////////////
ClassType*
NativeTypeSystem::get_unnamed_class_type(size_t bitSize, bool isUnion)
{
    RefPtr<SharedString> name = isUnion ? unnamed_union() : unnamed_type();
    ClassType* type = new ClassTypeImpl(this, name.get(), bitSize, isUnion);
    manage(type);
    return type;
}



////////////////////////////////////////////////////////////////
DebugSymbol* NativeTypeSystem::create_debug_symbol
(
    DebugInfoReader* reader,
    Thread* thread,
    DataType* type,
    SharedString* name,
    addr_t addr,
    SharedString* declFile,
    size_t declLine,
    bool isRetValue
) const
{
    return DebugSymbolImpl::create(reader,
            *CHKPTR(thread),
            *CHKPTR(type),
            *CHKPTR(name),
            addr,
            declFile,
            declLine,
            isRetValue).detach();
}


////////////////////////////////////////////////////////////////
DataType*
NativeTypeSystem::get_complex_float(SharedString* name, size_t nbytes)
{
    if (DataType* type = get_float_type(NULL, nbytes / 2))
    {
        return manage(new ComplexImpl(name, *type));
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
DataType*
NativeTypeSystem::get_complex_int(SharedString* name, size_t nbytes)
{
    using Platform::byte_size;

    if (DataType* type = get_int_type(NULL, nbytes * byte_size / 2, true))
    {
        return manage(new ComplexImpl(name, *type));
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
DataType*
NativeTypeSystem::get_ptr_to_member_type(DataType* base, DataType* type)
{
    if (base && type)
    {
        return manage(new PtrToMemberTypeImpl(*this, *base, *type));
    }
    return NULL;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
