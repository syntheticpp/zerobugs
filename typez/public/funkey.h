#ifndef FUNKEY_H__72334DC6_4827_4F04_B0BD_F454C21EC24C
#define FUNKEY_H__72334DC6_4827_4F04_B0BD_F454C21EC24C
//
// $Id: funkey.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <algorithm> // for lexicographical_compare
#include <cassert>
#include <vector>
#include "zdk/data_type.h"


namespace detail
{
    typedef std::vector<const DataType*> DataVect;

    inline bool ZDK_LOCAL
    lexicographical_compare(DataVect::const_iterator __first1,
                            DataVect::const_iterator __last1,
                            DataVect::const_iterator __first2,
                            DataVect::const_iterator __last2)
    {
      const size_t __len1 = std::distance(__first1, __last1) * sizeof (DataType*);
      const size_t __len2 = std::distance(__first2, __last2) * sizeof (DataType*);

      const int __result = memcmp(&*__first1, &*__first2, std::min(__len1, __len2));
      return __result != 0 ? (__result < 0) : (__len1 < __len2);
    }
}


class ZDK_LOCAL FunTypeKey
{
public:
    FunTypeKey( const DataType& retType,
                DataType* const* argTypes,
                size_t argCount,
                bool varArgs,
                bool strict)
        : varArgs_(varArgs), strict_(strict)
    {
        assert(argTypes || argCount == 0);
        prototype_.reserve(1 + argCount);
        prototype_.push_back(&retType);

        for (size_t i = 0; i != argCount; ++i, ++argTypes)
        {
            prototype_.push_back(*argTypes);
        }
    }

    bool operator<(const FunTypeKey& other) const
    {
        if (varArgs_ < other.varArgs_)
        {
            return true;
        }
        if (strict_ < other.strict_)
        {
            return true;
        }
        if (prototype_.empty() && !other.prototype_.empty())
        {
            return true;
        }
        if (other.prototype_.empty() && !prototype_.empty())
        {
            return false;
        }
        return detail::lexicographical_compare(
            prototype_.begin(), prototype_.end(),
            other.prototype_.begin(), other.prototype_.end());
    }

    bool operator==(const FunTypeKey& other) const
    {
        return varArgs_ == other.varArgs_
            && strict_ == other.strict_
            && prototype_.size() == other.prototype_.size()
            && std::equal(prototype_.begin(), prototype_.end(), other.prototype_.begin());
    }

private:
    // function prototype: the type id of the return
    // type, followed by argument type ids.
    std::vector<const DataType*> prototype_;
    bool varArgs_;
    bool strict_;
};

#endif // FUNKEY_H__72334DC6_4827_4F04_B0BD_F454C21EC24C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
