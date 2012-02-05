#ifndef TYPE_TAGS_H__64E17FF9_636A_4AD9_A2B6_EFDC821CBF1B
#define TYPE_TAGS_H__64E17FF9_636A_4AD9_A2B6_EFDC821CBF1B
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
#include "zdk/buffer.h"
#include "zdk/interface_cast.h"
#include "zdk/variant.h"
#include <assert.h>
#include <math.h>
#include <limits>
#include <memory>

template<typename T>
inline bool is_nan(T) { return false; }

static inline bool is_nan(double val)
{
    return std::isnan(val);
}

template<typename T> struct type_tag { };


template<typename T>
inline void put(Unknown2* unk,
                T val,
                Variant::TypeTag tag = type_tag<T>::tag)
{
    if (Buffer* buf = interface_cast<Buffer*>(unk))
    {
        buf->resize(sizeof (T));
        buf->put(&val, sizeof (T));
    #if 0
        assert(is_nan(val) ||
           fabs(*reinterpret_cast<const T*>(buf->data()) - val)
                <= std::numeric_limits<T>::epsilon());
    #endif
        if (Variant* v = interface_cast<Variant*>(unk))
        {
            v->set_type_tag(tag);
        }
    }
    else
    {
        assert(!unk);
    }
}


template<typename T>
inline void put(Variant* v,
                T val,
                Variant::TypeTag tag = type_tag<T>::tag)
{
    if (Buffer* buf = interface_cast<Buffer*>(v))
    {
        assert(v->type_tag() == Variant::VT_NONE);
        buf->resize(sizeof (T));
        buf->put(&val, sizeof (T));

        assert(*reinterpret_cast<const T*>(buf->data()) == val);
        v->set_type_tag(tag);
    }
    else
    {
        assert(!v);
    }
}

template<typename T>
struct type_tag<std::auto_ptr<T> >
{ };

template<> struct type_tag<bool>
{
    static const Variant::TypeTag tag = Variant::VT_BOOL;
};

template<> struct type_tag<int8_t>
{
    static const Variant::TypeTag tag = Variant::VT_INT8;
};

template<> struct type_tag<uint8_t>
{
    static const Variant::TypeTag tag = Variant::VT_UINT8;
};

template<> struct type_tag<int16_t>
{
    static const Variant::TypeTag tag = Variant::VT_INT16;
};

template<> struct type_tag<uint16_t>
{
    static const Variant::TypeTag tag = Variant::VT_UINT16;
};

template<> struct type_tag<int32_t>
{
    static const Variant::TypeTag tag = Variant::VT_INT32;
};

template<> struct type_tag<uint32_t>
{
    static const Variant::TypeTag tag = Variant::VT_UINT32;
};

#if (__WORDSIZE == 32)
template<> struct type_tag<long>
{
    static const Variant::TypeTag tag = Variant::VT_INT32;
};

template<> struct type_tag<unsigned long>
{
    static const Variant::TypeTag tag = Variant::VT_UINT32;
};
#endif

template<> struct type_tag<int64_t>
{
    static const Variant::TypeTag tag = Variant::VT_INT64;
};

template<> struct type_tag<uint64_t>
{
    static const Variant::TypeTag tag = Variant::VT_UINT64;
};

template<> struct type_tag<void*>
{
    static const Variant::TypeTag tag = Variant::VT_POINTER;
};

template<> struct type_tag<float>
{
    static const Variant::TypeTag tag = Variant::VT_FLOAT;
};

template<> struct type_tag<double>
{
    static const Variant::TypeTag tag = Variant::VT_DOUBLE;
};

template<> struct type_tag<long double>
{
    static const Variant::TypeTag tag = Variant::VT_LONG_DOUBLE;
};


#endif // TYPE_TAGS_H__64E17FF9_636A_4AD9_A2B6_EFDC821CBF1B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
