//
// $Id: variant.cpp 710 2010-10-16 07:09:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include <string>
#include <boost/static_assert.hpp>
#include "zdk/check_ptr.h"
#include "zdk/data_type.h"
#include "zdk/thread_util.h"
#include "zdk/types.h"
#include "zdk/variant_util.h"
#include "dharma/system_error.h"
#include "variant_impl.h"


#define incorrect_type() return throw_incorrect_type(tag_, __func__)


static int
throw_incorrect_type(Variant::TypeTag tag, const char* func)
{
    std::ostringstream err;
    err << func << ": incorrect type, tag=" << tag;
    throw std::logic_error(err.str());
}



VariantImpl::VariantImpl()
    : tag_(VT_NONE), enc_(VE_BINARY), buf_(new BufferImpl)
{
}


VariantImpl::VariantImpl(DebugSymbol& symbol)
    : tag_(VT_NONE)
    , enc_(VE_BINARY)
    , buf_(new BufferImpl)
    , sym_(&symbol)
{
    const DataType* type = CHKPTR(symbol.type());

    if (!symbol.value())
    {
        throw std::runtime_error("symbol has null value");
    }
    if (type)
    {
        if (type->parse(symbol.value()->c_str(), this))
        {
            assert(tag_ != VT_NONE); // parse postcondition
        }
        else if (tag_ == VT_NONE)
        {
            tag_ = VT_VOID;
        }
    }
}


VariantImpl::~VariantImpl() throw()
{
}


void VariantImpl::set_type_tag(TypeTag tag)
{
    assert(tag_ == VT_NONE);
    tag_ = tag;
}


Variant::TypeTag VariantImpl::type_tag() const
{
    return tag_;
}


size_t VariantImpl::size() const
{
    assert(buf_.get());
    assert(buf_->size() || tag_ == VT_OBJECT || tag_ == VT_NONE);

    return buf_->size();
}


uint64_t VariantImpl::uint64() const
{
    switch (tag_)
    {
    case VT_INT8:
        throw std::domain_error("converting signed to unsigned");
        //assert(size() == sizeof(int8_t));
        //return *reinterpret_cast<const int8_t*>(data());

    case VT_UINT8:
        assert(size() == sizeof(uint8_t));
        return *reinterpret_cast<const uint8_t*>(data());

    case VT_INT16:
        throw std::domain_error("converting signed to unsigned");
        //assert(size() == sizeof(int16_t));
        //return *reinterpret_cast<const int16_t*>(data());

    case VT_UINT16:
        assert(size() == sizeof(uint16_t));
        return *reinterpret_cast<const uint16_t*>(data());

    case VT_UINT32:
        assert(size() == sizeof(uint32_t));
        return *reinterpret_cast<const uint32_t*>(data());

    case VT_INT32:
        throw std::domain_error("converting signed to unsigned");
        //assert(size() == sizeof(int32_t));
        //return *reinterpret_cast<const int32_t*>(data());

    case VT_UINT64:
        assert(size() == sizeof(uint64_t));
        return *reinterpret_cast<const uint64_t*>(data());

    // forbid conversions here, let the client code do it explicitly
    case VT_INT64:
        throw std::domain_error("converting signed to unsigned");
        break;

    case VT_FLOAT:
    case VT_DOUBLE:
    case VT_LONG_DOUBLE:
        throw std::domain_error("converting float to integer");
        break;

    default:
        incorrect_type();
    }
}


int64_t VariantImpl::int64() const
{
    switch (tag_)
    {
    case VT_INT8:
        assert(size() == sizeof(int8_t));
        return *reinterpret_cast<const int8_t*>(data());

    case VT_UINT8:
        assert(size() == sizeof(uint8_t));
        return *reinterpret_cast<const uint8_t*>(data());

    case VT_INT16:
        assert(size() == sizeof(int16_t));
        return *reinterpret_cast<const int16_t*>(data());

    case VT_UINT16:
        assert(size() == sizeof(uint16_t));
        return *reinterpret_cast<const uint16_t*>(data());

    case VT_UINT32:
        assert(size() == sizeof(uint32_t));
        return *reinterpret_cast<const uint32_t*>(data());

    case VT_INT32:
        assert(size() == sizeof(int32_t));
        return *reinterpret_cast<const int32_t*>(data());

    case VT_INT64:
        assert(size() == sizeof(int64_t));
        return *reinterpret_cast<const int64_t*>(data());

    case VT_UINT64:
        // forbid conversion
        throw std::domain_error("converting unsigned to signed");
        break;

    case VT_FLOAT:
    case VT_DOUBLE:
    case VT_LONG_DOUBLE:
        throw std::domain_error("converting float to integer");
        break;

    default:
        incorrect_type();
    }
}


long double VariantImpl::long_double() const
{
    switch (tag_)
    {
    case VT_FLOAT:
        assert(size() == sizeof(float));
        return *reinterpret_cast<const float*>(data());

    case VT_DOUBLE:
        assert(size() == sizeof(double));
        return *reinterpret_cast<const double*>(data());

    case VT_LONG_DOUBLE:
        assert(size() == sizeof(long double));
        return *reinterpret_cast<const long double*>(data());
        break;

    // forbid implicit conversions
    case VT_INT8:
    case VT_UINT8:
    case VT_INT16:
    case VT_UINT16:
    case VT_UINT32:
    case VT_INT32:
    case VT_INT64:
        throw std::domain_error("converting integer to float");

    default:
        incorrect_type();
    }
}


addr_t VariantImpl::pointer() const
{
    if (tag_ != VT_POINTER)
    {
        incorrect_type();
    }

    addr_t result = 0;

    if (encoding() == VE_STRING)
    {
        BOOST_STATIC_ASSERT(sizeof(addr_t) == sizeof(long));
        result = strtoul((const char*)data(), 0, 0);
        if (result == ULONG_MAX && errno)
        {
            throw SystemError(__func__, errno);
        }
    }
    else if (buf_->size() == sizeof(int))
    {
        result = *reinterpret_cast<const int*>(data());
    }
    else
    {
        assert(buf_->size() >= sizeof(addr_t));
        result = *reinterpret_cast<const addr_t*>(data());
    }

    if (result == 0)
    {
        if (sym_)
        {
            result = sym_->addr();
            if (result && sym_->thread())
            {
                thread_read(*sym_->thread(), result, result);
            }
        }
    }
    return result;
}


DebugSymbol* VariantImpl::debug_symbol() const
{
    return sym_.get();
}


const void* VariantImpl::data() const
{
    return buf_->data();
}


uint64_t VariantImpl::bits() const
{
    switch (tag_)
    {
    case VT_BOOL:
        assert(size() == sizeof(bool));
        return *reinterpret_cast<const bool*>(data());

    case VT_INT8:
        assert(size() == sizeof(int8_t));
        return *reinterpret_cast<const int8_t*>(data());

    case VT_UINT8:
        assert(size() == sizeof(uint8_t));
        return *reinterpret_cast<const uint8_t*>(data());

    case VT_INT16:
        assert(size() == sizeof(int16_t));
        return *reinterpret_cast<const int16_t*>(data());

    case VT_UINT16:
        assert(size() == sizeof(uint16_t));
        return *reinterpret_cast<const uint16_t*>(data());

    case VT_UINT32:
        assert(size() == sizeof(uint32_t));
        return *reinterpret_cast<const uint32_t*>(data());

    case VT_INT32:
        assert(size() == sizeof(int32_t));
        return *reinterpret_cast<const int32_t*>(data());

    case VT_UINT64:
    case VT_INT64:
        assert(size() == sizeof(uint64_t));
        return *reinterpret_cast<const uint64_t*>(data());

    case VT_FLOAT:
    case VT_DOUBLE:
    case VT_LONG_DOUBLE:
        throw std::domain_error("converting float to bits");
        break;

    default:
        incorrect_type();
    }
}


void VariantImpl::copy(const Variant* other, bool lvalue)
{
    assert(other);

    if (other)
    {
        resize(other->size());
        buf_->put(other->data(), other->size(), 0);

        tag_ = other->type_tag();
        if (lvalue)
        {
            sym_ = other->debug_symbol();
        }
    }
}


void VariantImpl::set_debug_symbol(DebugSymbol* symbol)
{
    assert(!sym_);
    sym_ = symbol;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
