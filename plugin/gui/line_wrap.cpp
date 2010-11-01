// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: line_wrap.cpp 714 2010-10-17 10:03:52Z root $
//
#include <cassert>
#include <vector>
#include "line_wrap.h"

std::string line_wrap(const std::string& message, size_t maxw)
{
    if (message.length() < maxw)
    {
        return message;
    }
    else
    {
        const size_t length = message.length();
        const size_t count = length / maxw;
        std::vector<std::string> lines(count);

        for (size_t i = 0; i != count; ++i)
        {
            lines[i] = message.substr(maxw * i, maxw);
        }
        std::string result;
        for (size_t i = 0;;)
        {
            result += lines[i];
            if (++i == count)
            {
                if ((i = length % maxw) != 0)
                {
                    result += "\n";
                    result += message.substr(length - i);
                }
                break;
            }
            result += "\n";
        }
        return result;
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
