//
// $Id: canonical_path_unix.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <limits.h>
#include <stdlib.h>
#include <vector>
#include "canonical_path.h"
#include "zdk/shared_string_impl.h"


std::string canonical_path(const char* path)
{
    char buf[PATH_MAX] = { 0 };

    std::string result;

    if ((::realpath(path, &buf[0]) == 0) && path)
    {
        result = path;
    }
    else
    {
        result = &buf[0];
    }

    return result;
}



char* abspath(const char* path)
{
    char buf[PATH_MAX] = { 0 };
    char* result = NULL;

    if (path && (::realpath(path, buf) == 0))
    {
        result = strdup(path);
    }
    else
    {
        result = strdup(buf);
    }
    return result;
}


RefPtr<SharedString> abspath(const RefPtr<SharedString>& path)
{
    char buf[PATH_MAX] = { 0 };

    RefPtr<SharedString> result;

    if (path && (::realpath(path->c_str(), buf) == 0))
    {
        result = path;
    }
    else
    {
        result = SharedStringImpl::take_ownership(strdup(buf));
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
