#ifndef ATTR_H__9D8F03FD_B5D8_4F5F_B08B_670F39458D7E
#define ATTR_H__9D8F03FD_B5D8_4F5F_B08B_670F39458D7E
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: attr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <string>
#include <vector>
#include "rpc/attr_enums.h"
#include "rpc/attr_names.h"
#include "rpc/msg.h"
#include "zdk/stream.h"


namespace RPC
{
    template<AttrEnum attr, typename T>
    struct ZDK_LOCAL Value
    {
        T val_;

        Value() : val_(T()) { }
        template<typename U> void assign(U val) { val_ = val; }

        Value& operator=(const Value& other)
        {
            val_ = other.val_;
            return *this;
        }
    };

    template<AttrEnum attr, typename T>
    inline ZDK_LOCAL
    T& value(Value<attr, T>& v) { return v.val_; }

    template<AttrEnum attr, typename T>
    inline ZDK_LOCAL
    const T& value(const Value<attr, T>& v) { return v.val_; }

    template<AttrEnum attr, typename T, typename Base> class Attr;


    template<AttrEnum attr, typename Base>
    class ZDK_LOCAL Attr<attr, word_t, Base>
        : public Base
        , public Value<attr, word_t>
    {
    protected:
        virtual ~Attr() throw() { }

        void on_word(const char* name, word_t val)
        {
            if (strcmp(name, attr_name[attr]) == 0)
            {
                value<attr>(*this) = val;
            }
            else
            {
                Base::on_word(name, val);
            }
        }

        size_t write(OutputStream* out) const
        {
            size_t bytesWritten = Base::write(out);
            if (out)
            {
                bytesWritten += out->write_word(attr_name[attr], value<attr>(*this));
            }
            return bytesWritten;
        }
    };


    template<RPC::AttrEnum attr, typename Base>
    class ZDK_LOCAL Attr<attr, std::string, Base>
        : public Base
        , public Value<attr, std::string>
    {
    protected:
        virtual ~Attr() throw() { }

        void on_string(const char* name, const char* val)
        {
            if (strcmp(name, attr_name[attr]) == 0)
            {
                value<attr>(*this) = val;
            }
            else
            {
                Base::on_string(name, val);
            }
        }

        size_t write(OutputStream* out) const
        {
            size_t bytesWritten = Base::write(out);
            if (out)
            {
                bytesWritten += out->write_string(attr_name[attr],
                                                  value<attr>(*this).c_str());
            }
            return bytesWritten;
        }
    };


    template<RPC::AttrEnum attr, typename Base>
    class ZDK_LOCAL Attr<attr, std::vector<uint8_t>, Base>
        : public Base
        , public Value<attr, std::vector<uint8_t> >
    {
    protected:
        virtual ~Attr() throw() { }

        void on_bytes(const char* name, const void* bytes, size_t len)
        {
            if (strcmp(name, attr_name[attr]) == 0)
            {
                const uint8_t* pb = static_cast<const uint8_t*>(bytes);
                value<attr>(*this).resize(len);
                value<attr>(*this).assign(pb, pb + len);
            }
            else
            {
                Base::on_bytes(name, bytes, len);
            }
        }

        size_t write(OutputStream* out) const
        {
            size_t bytesWritten = Base::write(out);
            if (out)
            {
                bytesWritten += out->write_bytes(
                    attr_name[attr],
                    &value<attr>(*this)[0],
                    value<attr>(*this).size());
            }
            return bytesWritten;
        }

    public:
        const std::vector<uint8_t>& bits() const { return this->val_; }
        std::vector<uint8_t>& bits() { return this->val_; }

        template<typename T>
        void put(const T& obj)
        {
            this->val_.resize(sizeof obj);
            memcpy(&this->val_[0], &obj, sizeof obj);
        }

        template<typename T>
        void get(T& obj)
        {
            assert(this->val_.size() == sizeof obj);
            memcpy(&obj, &this->val_[0], sizeof obj);
        };
    };
}

#endif // ATTR_H__9D8F03FD_B5D8_4F5F_B08B_670F39458D7E
