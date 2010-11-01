#ifndef CACHE_H__F6ACF857_FA74_4D77_857A_0FBAC57F2F97
#define CACHE_H__F6ACF857_FA74_4D77_857A_0FBAC57F2F97
//
// $Id: cache.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <map>
#include "comp_str.h"
#include "interface.h"
#include "wrapper.h"

namespace Dwarf
{

template<typename T> CLASS Cache
{
    Cache(const Cache&); // non-copyable
    Cache& operator=(const Cache&); // non-assignable

public:
    typedef WrapperTraits<T> traits;
    typedef Wrapper<T> value_type;
    typedef boost::shared_ptr<value_type> pointer;

    typedef std::multimap<const char*, pointer, StrLess> map_type;

    typedef typename map_type::const_iterator const_iterator;
    typedef std::pair<const_iterator, const_iterator> range;

    explicit Cache(Dwarf_Debug dbg) : dbg_(dbg), obj_(0)
    {
        Dwarf_Signed cnt = 0;
        Dwarf_Error  err = 0;

        if (traits::objects(
                dbg_,
                &obj_,
                &cnt,
                &err) == DW_DLV_ERROR)
        {
            throw Error(__func__, dbg_, err);
        }

        for (int i = 0; i != cnt; ++i)
        {
            // The wrapper takes ownership of obj_[i],
            // and will deallocate it in its dtor.
            pointer ptr(new value_type(dbg, obj_[i]));

            map_.insert(make_pair(ptr->name(), ptr));
        }
    }

    ~Cache() throw()
    {
        if (obj_)
        {
            dwarf_dealloc(dbg_, obj_, DW_DLA_LIST);
        }
    }

    range lookup(const char* name) const
    {
        assert(name);
        return map_.equal_range(name);
    }

    range all() const
    {
        return range(map_.begin(), map_.end());
    }

private:
    Dwarf_Debug dbg_;
    T*          obj_;
    map_type    map_;
};

} // namespace Dwarf

#endif // CACHE_H__F6ACF857_FA74_4D77_857A_0FBAC57F2F97
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
