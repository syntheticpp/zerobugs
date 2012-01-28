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
#include <set>
#include <boost/utility.hpp>
#include "zdk/check_ptr.h"
#include "zdk/data_type.h"
#include "zdk/debug_symbol_list.h"
#include "zdk/types.h"
#include "public/type_names.h"

using namespace std;


typedef set<pair<RefPtr<SharedString>, addr_t> > SymSet;


namespace
{
    class InfiniteRecursivityGuard : boost::noncopyable
    {
        SymSet& set_;
        DebugSymbol *sym_;

    public:
        InfiniteRecursivityGuard(SymSet& set, DebugSymbol* sym)
            : set_(set), sym_(sym)
        { }

        ~InfiniteRecursivityGuard() throw()
        {
            set_.erase(make_pair(sym_->name(), sym_->addr()));
        }
    };
} // namespace


RefPtr<DebugSymbol>
lookup_child(DebugSymbol& obj, const char* name, SymSet& symset)
{
    if (!symset.insert(make_pair(obj.name(), obj.addr())).second)
    {
        return NULL;
    }
    InfiniteRecursivityGuard guard(symset, &obj);

    if (!obj.value())
    {
        obj.read(NULL);
    }

    DebugSymbolList objects;
    ClassType* klass = interface_cast<ClassType*>(obj.type());

    const size_t count = obj.enum_children(NULL);

    for (size_t n = 0; n != count; ++n)
    {
        RefPtr<DebugSymbol> child = obj.nth_child(n);

        if (!child->name())
        {
            continue;
        }
        if (child->name()->is_equal(name))
        {
            return child;
        }

        // build a list of anonymous members and base classes
        // to recurse into
        if (klass)
        {
            if (child->name()->is_equal2(unnamed_type()))
            {
                assert (interface_cast<ClassType*>(child->type()));
                objects.push_back(child);
            }
            else if (child->type()
                  && klass->lookup_base(child->type()->name(), NULL, true))
            {
                objects.push_back(child);
            }
        }
        else if (interface_cast<ClassType*>(child->type()))
        {
            objects.push_back(child);
        }
    }
    // now dive into base classes
    for (DebugSymbolList::const_iterator i = objects.begin();
         i != objects.end();
         ++i)
    {
        if (RefPtr<DebugSymbol> child = lookup_child(**i, name, symset))
        {
            return child;
        }
    }
    return NULL;
}


/**
 * Given a debug symbol associated with a C++ obj or struct,
 * lookup recursively for a child, by name
 */
RefPtr<DebugSymbol> lookup_child(DebugSymbol& obj, const char* name)
{
    SymSet lookupSet;

    return lookup_child(obj, name, lookupSet);
}



// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
