#ifndef SYMKEY_H__100EAC2E_16C4_414A_9A2F_9BAC40D91502
#define SYMKEY_H__100EAC2E_16C4_414A_9A2F_9BAC40D91502
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
#include <iosfwd>
#include <memory>
#include "zdk/platform.h"
#include "zdk/ref_ptr.h"
#include "zdk/weak_ptr.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"

class SharedString;
class DebugSymbol;


class SymKey
{
public:
    explicit SymKey(DebugSymbol&);

    SymKey() : addr_(0), depth_(0), hash_(0) { }
  /*
    SymKey(const SymKey& other)
        : addr_(other.addr_)
        , depth_(other.depth_)
        , hash_(other.hash_)
        , name_(other.name_)
    { } */

    ~SymKey() { }

    friend bool operator<(const SymKey&, const SymKey&);
    friend bool operator==(const SymKey&, const SymKey&);

    void print(std::ostream&) const;
    typedef Platform::addr_t addr_t;

    uint32_t hash() const { return hash_; }

private:
    addr_t   addr_;
    uint32_t depth_;
    uint32_t hash_;
    WeakPtr<SharedString> name_;
    //RefPtr<SharedString> name_;
};


bool operator<(const SymKey&, const SymKey&);
bool operator==(const SymKey&, const SymKey&);

inline std::ostream&
operator<<(std::ostream& out, const SymKey& key)
{
    key.print(out);
    return out;
}

namespace __gnu_cxx
{
    template<> struct hash<SymKey>
    {
        int operator()(const SymKey& key) const
        {
            return key.hash();
        }
    };
}

#endif // SYMKEY_H__100EAC2E_16C4_414A_9A2F_9BAC40D91502
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
