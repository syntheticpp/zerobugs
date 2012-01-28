#ifndef VARIANT_REG_H__D9DDDDF4_751C_4C95_80B1_8F8947398086
#define VARIANT_REG_H__D9DDDDF4_751C_4C95_80B1_8F8947398086
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

#include "zdk/stdexcept.h"
#include "zdk/type_tags.h"
#include "zdk/zobject_impl.h"


/**
 * A read-only, light-weight Variant.
 */
template<typename T>
class VariantLite : public ZObjectImpl<Variant>
{
    void illegal_operation(const std::string& fun) const
    {
        throw std::logic_error(fun + ": illegal operation");
    }

public:

BEGIN_INTERFACE_MAP(VariantLite)
    INTERFACE_ENTRY(Variant)
END_INTERFACE_MAP()

    explicit VariantLite(T value)
        : tag_(::type_tag<T>::tag)
        , value_(value)
    { }

    virtual ~VariantLite() throw() { }

    void set_type_tag(TypeTag)
    { illegal_operation(__func__); }

    virtual TypeTag type_tag() const { return tag_; }

    virtual size_t size() const { return sizeof(T); }

    virtual uint64_t uint64() const
    { return static_cast<uint64_t>(value_); }

    virtual int64_t int64() const
    { return static_cast<int64_t>(value_); }

    virtual long double long_double() const
    { return static_cast<long double>(value_); }

    virtual const void* data() const
    { return &value_; }

    virtual Platform::addr_t pointer() const
    { illegal_operation(__func__); return 0; }

    virtual uint64_t bits() const
    { return static_cast<uint64_t>(value_); }

    virtual DebugSymbol* debug_symbol() const
    { illegal_operation(__func__); return 0; }

    virtual void copy(const Variant* v, bool)
    {
        if (v)
        {
            if (is_integer(*v))
            {
                value_ = static_cast<T>(v->int64());
            }
            else if (is_float(*v))
            {
                value_ = static_cast<T>(v->long_double());
            }
            else
            {
                illegal_operation(__func__);
            }
        }
    }

    virtual Encoding encoding() const { return VE_BINARY; }

private:
    TypeTag tag_;
    T       value_;
};

#endif // VARIANT_REG_H__D9DDDDF4_751C_4C95_80B1_8F8947398086
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
