// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <iostream>
#include "zdk/uuid.h"


std::ostream& operator<<(std::ostream& os, const ZDK_UUID& uuid)
{
    char tmp[37];
    uuid_to_string(&uuid, tmp);

    return os << &tmp[0];
}


std::ostream& operator<<(std::ostream& os, const uuidref_t& uuid)
{
    char tmp[37];
    uuid_to_string(uuid, tmp);

    return os << &tmp[0];
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
