#ifndef EXEC_ARGS_H__91DB4983_A5BD_4F63_8F0B_4D33593D8C58
#define EXEC_ARGS_H__91DB4983_A5BD_4F63_8F0B_4D33593D8C58
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

#include "dharma/sarray.h"
#include <boost/tokenizer.hpp>

/**
 * Breaks down a command line into a vector of args.
 */
class ZDK_LOCAL ExecArg : public SArray
{
    typedef boost::escaped_list_separator<char> Separator;

    typedef boost::tokenizer<Separator> Tokenizer;

public:
    ExecArg() { }
    ExecArg(const ExecArg& other) : SArray(other)
    { // do not copy cmd_
    }

    explicit ExecArg(const std::string& cmd);

    template<typename Iter>
    ExecArg(Iter first, Iter last) : SArray(first, last)
    {
    }

    const std::string& command_line() const;

private:
    mutable std::string cmd_;
};
#endif // EXEC_ARGS_H__91DB4983_A5BD_4F63_8F0B_4D33593D8C58
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
