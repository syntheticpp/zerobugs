//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// $Id: uuid.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <cstdio>
#include <iostream>
#include "zdk/uuid.h"

ZDK_UUID uuid_from_string(const char* str)
{
    ZDK_UUID uuid /* = { 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0 } } */;

    char* p = 0;
    uuid.data1 = strtoul(str, &p, 16);

    assert(*p == '-');
    ++p;
    uuid.data2 = strtoul(p, &p, 16);

    assert(*p == '-');
    ++p;
    uuid.data3 = strtoul(p, &p, 16);

    assert(*p == '-');
    ++p;
    uuid.data4 = strtoul(p, &p, 16);

    assert(*p == '-');
    ++p;
    char tmp[4] = { 0, 0, 0, 0 };

    for (int i=0; i < 6; ++i)
    {
        assert(*p);
        tmp[0] = *p++;
        tmp[1] = *p++;

        uuid.data5[i] = strtoul(tmp, 0, 16);
    }

    return uuid;
}

/*
ZDK_UUID* new_uuid_from_string(const char* str)
{
    ZDK_UUID* uuid = new ZDK_UUID;

    char* p = 0;
    uuid->data1 = strtoul(str, &p, 16);

    assert(*p == '-');
    ++p;
    uuid->data2 = strtoul(p, &p, 16);

    assert(*p == '-');
    ++p;
    uuid->data3 = strtoul(p, &p, 16);

    assert(*p == '-');
    ++p;
    uuid->data4 = strtoul(p, &p, 16);

    assert(*p == '-');
    ++p;
    char tmp[4] = { 0, 0, 0, 0 };

    for (int i=0; i < 6; ++i)
    {
        assert(*p);
        tmp[0] = *p++;
        tmp[1] = *p++;

        uuid->data5[i] = strtoul(tmp, 0, 16);
    }
    return uuid;
}
*/


void uuid_to_string(uuidref_t uuid, char(&output)[37])
{
    assert(uuid);

    snprintf(output, sizeof(output),
        "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uuid->data1, uuid->data2, uuid->data3,
        uuid->data4 >> 8, uuid->data4 & 0xFF,
        uuid->data5[0], uuid->data5[1], uuid->data5[2],
        uuid->data5[3], uuid->data5[4], uuid->data5[5]);
}


/*
int uuid_compare(uuidref_t lhs, uuidref_t rhs)
{
    return memcmp(lhs, rhs, sizeof(ZDK_UUID));
}
*/

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
