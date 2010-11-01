// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include "zdk/uuid.h"

char str[] = "c8b602e3-ec01-4936-8413-ec379932e92b";

int main()
{
    ZDK_UUID uuid = uuid_from_string(str);

    char buf[37];
    uuid_to_string(&uuid, buf);

    std::cout << str << std::endl;
    std::cout << buf << std::endl;

    assert(strcmp(buf, str) == 0);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
