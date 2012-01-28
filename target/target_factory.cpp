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
#include "generic/less.h"
#include "target/target.h"
#include "target/target_factory.h"

using namespace std;


bool operator<(const TargetFactory::Key& lhs,
               const TargetFactory::Key& rhs)
{
    typedef TargetFactory::Key Key;

    return Less::pred(&Key::osid_,
            Less::pred(&Key::wordsize_,
             Less::pred(&Key::live_,
              Less::pred(&Key::subsystem_, Less::End()))))(lhs, rhs);
}


bool
TargetFactory::register_target(OSID osid,
                               unsigned wordsize,
                               bool live,
                               const string& subsystem,
                               FuncPtr func,
                               bool override)
{
    Key key = { osid, wordsize, live, subsystem };

    pair<map_type::iterator, bool> result =
        map_.insert(make_pair(key, func));

    if (!result.second && override)
    {
        result.first->second = func;
        result.second = true;
    }
    return result.second;
}


RefPtr<Target>
TargetFactory::create_target(OSID osid,
                             unsigned wordsize,
                             bool live,
                             const string& subsystem,
                             debugger_type& dbg)
{
    RefPtr<Target> target;

    Key key = { osid, wordsize, live, subsystem };
    map_type::const_iterator i = map_.find(key);

    if (i != map_.end())
    {
        target = (i->second)(dbg);
    }
    return target;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
