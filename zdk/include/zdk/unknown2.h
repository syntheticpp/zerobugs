#ifndef UNKNOWN2_H__0AD0D453_3555_4A99_87E0_85D3452BCA12
#define UNKNOWN2_H__0AD0D453_3555_4A99_87E0_85D3452BCA12
//
// $Id: unknown2.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// This file contains the definition of the base interfaces
// Unknown and Unknown2.
// For inter-compiler compatibility, the COM format for vtables
// is used.  This is the default format in the g++ 3.2.x and above
// ABI, but must be explicitly specified for g++ 2.95 with the
// GNU specific __attribute__((com_interface)) keywords. The
// attribute applies to all classes in a hierarchy; the purpose of
// Unknown is just to serve as a base for the interface hierarchy.
// A side-effect of the com_interface attribute is that it turns
// off the dynamic_cast language feature, and rtti (runtime-type-info)
// needs to be implemented manually -- and this is exactly the purpose
// for query_interface() and Unknown2.

#include <assert.h>

#include "zdk/export.h"
#include "zdk/uuid.h"

#if defined(__GNUC__) && (__GNUC__ < 3) && !defined(__INTEL_COMPILER)
 #define ZDK_VTABLE __attribute__((com_interface))
 #define ZDK_INLINE inline
#else
// GCC 3.2 and above generate COM-compatible vtables by default
 #define ZDK_VTABLE

 #if (__GNUC__ == 3) && (__GNUC_MINOR__ == 4)
 // gcc3.4 workaround: "sorry, unimplemented: inlining failed in call..."
  #define ZDK_INLINE inline
 #else
  #define ZDK_INLINE inline __attribute__((always_inline))
 #endif
#endif

#define ZDK_PACKED __attribute__((packed)) ZDK_EXPORT

#define DECLARE_ZDK_INTERFACE(i) struct ZDK_LOCAL ZDK_VTABLE i

#if 0
 #define DECLARE_ZDK_INTERFACE_(i,b) struct ZDK_LOCAL i : public b
#else
template<typename U, typename V>
struct TypeId : public V { static const char* _type(); };

#define TYPEID(i) struct i; \
    template<typename V> struct ZDK_LOCAL TypeId<i, V> : public V \
    { static const char* _type() { return #i; } };

#define DECLARE_ZDK_INTERFACE_(i,b) TYPEID(i) \
    struct ZDK_LOCAL i : public TypeId<i, b>
#endif


DECLARE_ZDK_INTERFACE(Unknown)
{
#if defined (DEBUG) && defined (__INTEL_COMPILER)
    // silence off warning about non-virtuald dtor
    virtual ~Unknown() { }
#endif
};


DECLARE_ZDK_INTERFACE(Unknown2)
{
#if defined (DEBUG) && defined (__INTEL_COMPILER)
    // silence off warning about non-virtuald dtor
    virtual ~Unknown2() { }
#endif

    virtual bool query_interface(uuidref_t, void**) = 0;

    virtual const char* _name() const { return "Unknown2"; }
};

template<typename T>
inline bool ZDK_LOCAL
query_interface_aggregate(uuidref_t iid, T ptr, void** p)
{
    if (ptr)
    {
        if (uuid_equal(iid, ptr->_uuid()))
        {
            *p = ptr;
            return true;
        }
    }
    return false;
}

/**
 * @note delegate is DEEP, aggregate is SHALLOW; should probably
 * change the function and macro names, since this is just an
 * arbitrary choice of words...
 */
template<typename T>
inline bool ZDK_LOCAL
query_interface_delegate(uuidref_t iid, T ptr, void** p)
{
    if (ptr)
    {
        if (ptr->query_interface(iid, p))
        {
            return true;
        }
    }
    return false;
}
//
// Associate a unique universal identifier with this class
//
#define DECLARE_UUID(str) \
    static ZDK_INLINE ZDK_EXPORT uuidref_t _uuid() \
    { \
        static const ZDK_UUID uuid = uuid_from_string(str); \
        return &uuid; \
    }
//
// Query interface implementation macros
//
#define BEGIN_INTERFACE_MAP(X) \
    static const char* _type() ZDK_EXPORT { return #X; } \
    virtual const char* _name() const { return #X; }\
    bool query_interface(uuidref_t iid, void** p) { \
        assert(p); if (0) { }

#define BEGIN_INTERFACE_MAP_() \
    bool query_interface(uuidref_t iid, void** p) { \
		assert(p); if (0) { }

#define END_INTERFACE_MAP() return false; }

#define INTERFACE_ENTRY(X) else if (uuid_equal(iid, X::_uuid())) \
    { *p = static_cast<X*>(this); return true; }

#define INTERFACE_ENTRY_AGGREGATE(pm) \
    if (query_interface_aggregate(iid, pm, p)) return true;

#define INTERFACE_ENTRY_DELEGATE(pm) \
    if (query_interface_delegate(iid, pm, p)) return true;

#define INTERFACE_ENTRY_INHERIT(base) \
    if (base::query_interface(iid, p)) { return true; }

#define INTERFACE_ENTRY_HIDE(X) \
    else if (uuid_equal(iid, X::_uuid())) { return false; }

#endif // UNKNOWN2_H__0AD0D453_3555_4A99_87E0_85D3452BCA12
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
