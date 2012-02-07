#ifndef TYPEMAP_H__527252E2_95D0_459B_B422_BCA964CD2065
#define TYPEMAP_H__527252E2_95D0_459B_B422_BCA964CD2065
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

#include <libdwarf.h>               // for Dwarf_Off
#include <vector>
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "zdk/shared_string.h"
#include "typez/public/types.h"

#ifndef HAVE_HASH_MAP
 #error bummer!
#endif
class DataType; // forward

typedef std::pair<ino_t, Dwarf_Off> TypeOffset;

namespace __gnu_cxx
{
    template<> struct ZDK_LOCAL hash<TypeOffset>
    {
        int operator()(const TypeOffset& typeOffset) const
        {
            return ext::hash<int>()(typeOffset.second);
        }
    };
}


namespace Dwarf
{
    class Debug;

    /**
     * Caches types by name and by the index of the debug info entry
     */
    CLASS TypeMap
    {
        // types indexed by name
        typedef ext::hash_map<RefPtr<SharedString>, WeakDataTypePtr> ByName;

        // types indexed by the inode of the binary that contains
        // the debug info entry, and offset of the debug entry for
        // the type
        typedef ext::hash_map<TypeOffset, WeakDataTypePtr> ByOffset;

    public:
        TypeMap() { }

        ~TypeMap();

        RefPtr<DataType> add(const Dwarf::Die& die,
                             const RefPtr<DataType>& dataType,
                             bool indexByName = true)
        {
            return add_internal(die, dataType, indexByName);
        }

        RefPtr<DataType> add(const Dwarf::Type& type,
                             const RefPtr<DataType>& dataType,
                             bool indexByName = true);

        RefPtr<DataType> find(const char* name) const;

        RefPtr<DataType> find(const Dwarf::Die&) const;

        void clear();

    private:
        RefPtr<DataType> add_internal(const Dwarf::Die&,
                                      const RefPtr<DataType>&,
                                      bool indexByName);

        RefPtr<DataType> find(const RefPtr<SharedString>& name) const;

        ByName   byName_;
        ByOffset byOffset_;
    };

} // namespace Dwarf
#endif // TYPEMAP_H__527252E2_95D0_459B_B422_BCA964CD2065
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
