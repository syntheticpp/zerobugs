#ifndef SYMBOL_H__D0A6A365_D66B_441A_AC0C_34F01D5E9BE4
#define SYMBOL_H__D0A6A365_D66B_441A_AC0C_34F01D5E9BE4
//
// $Id: symbol.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "public/elfW.h"

namespace ELF
{
    class Symbol
    {
    public:
        Symbol(const char* name = NULL,
               ElfW(Addr) addr = 0,
               ElfW(Xword) size = 0,
               unsigned char info = 0
              )
            : name_(name)
            , addr_(addr)
            , size_(size)
            , info_(info)
        { }

        const char* name() const { return name_; }

        ElfW(Addr) value() const { return addr_; }

        ElfW(Xword) size() const { return size_; }

        /// @todo: return enumerated type?
        unsigned char bind() const
        {
            return ELF32_ST_BIND(info_); // ELF32_ST_BIND is same as ELF64_ST_BIND
        }

        /// @todo: return enumerated type?
        unsigned char type() const
        {
            return ELF32_ST_TYPE(info_); // ELF32_ST_TYPE is same as ELF64_ST_TYPE
        }

        friend bool operator==(const Symbol&, const Symbol&);

    private:
        const char* name_; // points into string section's data
        ElfW(Addr) addr_;
        ElfW(Xword) size_;
        unsigned char info_;
    };


    bool inline operator==(const Symbol& lhs, const Symbol& rhs)
    {
        return lhs.addr_ == rhs.addr_ &&
               lhs.size_ == rhs.size_ &&
               lhs.info_ == rhs.info_ &&
               lhs.name_ == rhs.name_;
    }

    bool inline operator!=(const Symbol& lhs, const Symbol& rhs)
    {
        return !(lhs == rhs);
    }
}
#endif // SYMBOL_H__D0A6A365_D66B_441A_AC0C_34F01D5E9BE4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
