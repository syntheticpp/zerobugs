#ifndef PARSE_STATE_H__7A701ADE_AFE3_408C_9CF7_4F298E129E64
#define PARSE_STATE_H__7A701ADE_AFE3_408C_9CF7_4F298E129E64
//
// $Id: parse_state.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <algorithm>
#include <iosfwd>
#include <string>


/**
 * For debugging purposes, I keep a stack of parser
 * states. Unwinding and dumping the stack is very
 * useful for debugging the parser code.
 */
class ParseState
{
public:
    ParseState(const std::string& name, const char*& pos);
    ParseState(const ParseState& that)
        : name_(that.name_)
        , begin_(that.begin_)
        , end_(that.end_)
        , pos_(that.pos_)
    { }

    void swap(ParseState& that) throw()
    {
        name_.swap(that.name_);
        std::swap(begin_, that.begin_);
        std::swap(end_, that.end_);
        std::swap(pos_, that.pos_);
    }

    ParseState& operator=(ParseState that)
    {
        this->swap(that);
        return *this;
    }

    void freeze();

    std::ostream& dump(std::ostream& outs) const;

private:
    std::string     name_;
    const char*     begin_;
    const char*     end_;
    const char*&    pos_;
};

#endif // PARSE_STATE_H__7A701ADE_AFE3_408C_9CF7_4F298E129E64
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
