// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "test_common.h"
#include "dharma/system_error.h"
#include <errno.h>
#include <string.h>

#include <iostream>

int main()
{
    static const int errCode = 2;

    try
    {
        errno = errCode;

        // throw SystemError(errCode);
        throw SystemError();
    }
    catch (const SystemError& e)
    {
        std::cout << e.what() << std::endl;

        assert(strcmp(e.what(), strerror(errCode)) == 0);
        assert(e.error() == errCode);
    }

    return 0;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
