#ifndef ENUM_TYPE_H__1701543E_D716_4856_AB55_0F0FA3149DD8
#define ENUM_TYPE_H__1701543E_D716_4856_AB55_0F0FA3149DD8
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

#include <map>
#include <sstream>
#include "zdk/export.h"
#include "zdk/shared_string_impl.h"
#include "zdk/types.h"
#include "typez/public/data_type_impl.h"
#include "typez/public/type_tags.h"
#include "typez/public/value.h"

namespace detail
{
    template<typename T>
    void inline ZDK_LOCAL extract(std::istream& is, T& val)
    { is >> val; }

    template<>
    void inline ZDK_LOCAL extract<int8_t>(std::istream& is, int8_t& val)
    {
        int tmp = 0;
        is >> tmp;
        val = tmp;
    }

    template<>
    inline void ZDK_LOCAL extract<uint8_t>(std::istream& is, uint8_t& val)
    {
        int tmp = 0;
        is >> tmp;
        val = tmp;
    }
}


template<typename T = int>
class ZDK_LOCAL EnumTypeImpl : public DataTypeImpl<EnumType>
{
public:
    DECLARE_UUID("4f6843c1-65d8-4608-ba05-74a537ec40cf")

BEGIN_INTERFACE_MAP(EnumTypeImpl)
    INTERFACE_ENTRY(EnumTypeImpl)
    INTERFACE_ENTRY_INHERIT(DataTypeImpl<EnumType>)
END_INTERFACE_MAP()

    typedef T value_type;
    typedef std::map<T, RefPtr<SharedString> > map_type;

    EnumTypeImpl(SharedString* name, const map_type& m, bool f = false)
        : DataTypeImpl<EnumType>(name, sizeof(T) * Platform::byte_size)
        , map_(m)
        , fundamental_(f)
    {}

    ~EnumTypeImpl() throw() {}

protected:
    SharedString* read(DebugSymbol* sym, DebugSymbolEvents*) const
    {
        assert(sym);
        assert(sym->thread());

        Value<T> v;

        if (sym->is_return_value())
        {
            v = Value<T>(sym->addr());
        }
        else if (!sym->is_constant())
        {
            v.read(*sym);
        }
        typename map_type::const_iterator i(map_.find(v.value()));
        if (i != map_.end())
        {
            return i->second.get();
        }
        std::ostringstream tmp;
        tmp << v.value();
        return shared_string(tmp.str()).detach();
    }

    bool is_fundamental() const { return fundamental_; }

    std::string description() const
    {
        std::string result = DataTypeImpl<EnumType>::description();
        result += "\nEnumerated values:\n";
        typename map_type::const_iterator i = map_.begin();
        for (; i != map_.end(); ++i)
        {
            std::ostringstream tmp;
            tmp << i->second->c_str() << '=' << i->first << std::endl;
            result += tmp.str();
        }
        return result;
    }

    size_t parse(const char* value, Unknown2* unk) const
    {
        typename map_type::const_iterator i = map_.begin();
        for (; i != map_.end(); ++i)
        {
            const size_t len = i->second->length();
            if (strncmp(value, i->second->c_str(), len) == 0)
            {
                put(unk, i->first);
                return len;
            }
        }
        std::istringstream istr(value);
        istr.unsetf(std::ios_base::basefield);
        T v;
        detail::extract(istr, v);
        put(unk, v);
        return istr.tellg();
    }

    bool is_equal(const DataType* type) const
    {
        if (type)
        {
            return (bit_size() == type->bit_size())
                && interface_cast<const EnumTypeImpl<T>*>(type);
        }
        return false;
    }

private:
    map_type map_;
    bool fundamental_;
};

#endif // ENUM_TYPE_H__1701543E_D716_4856_AB55_0F0FA3149DD8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
