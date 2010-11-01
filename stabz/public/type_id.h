#ifndef TYPE_ID_H__713AF7AA_9464_4DAF_86D9_6EAB49777C72
#define TYPE_ID_H__713AF7AA_9464_4DAF_86D9_6EAB49777C72
//
// $Id: type_id.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <string>
#include <utility>

namespace Stab
{
    typedef std::pair<size_t, size_t> TypeID;

    std::string typeid_to_str(const TypeID& typeID);
}


#endif // TYPE_ID_H__713AF7AA_9464_4DAF_86D9_6EAB49777C72
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
