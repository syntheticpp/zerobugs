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
#include "exec_arg.h"
#include <boost/tokenizer.hpp>

typedef boost::escaped_list_separator<char> Separator;
typedef boost::tokenizer<Separator> Tokenizer;


ExecArg::ExecArg(const std::string& cmd) : cmd_(cmd)
{
    Separator delim('\\', ' ', '\"');
    Tokenizer tokenizer(cmd, delim);

    Tokenizer::const_iterator i = tokenizer.begin();

    for (; i != tokenizer.end(); ++i)
    {
        if (i->empty())
        {
            continue;
        }
        push_back(*i);
    }
}


const std::string& ExecArg::command_line() const
{
    if (cmd_.empty())
    {
        const std::deque<std::string>& s = strings();
        if (!s.empty())
        {
            std::deque<std::string>::const_iterator i = s.begin();
            for (;;)
            {
                cmd_ += '\"';
                cmd_ += *i;
                cmd_ += '\"';
                if (++i == s.end())
                {
                    break;
                }
                cmd_ += " ";
            }
        }
    }
    return cmd_;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
