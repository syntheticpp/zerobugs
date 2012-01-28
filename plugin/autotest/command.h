#ifndef TEST_H__3AE12423_C14E_44CC_8FF1_A5A5D8130EAB
#define TEST_H__3AE12423_C14E_44CC_8FF1_A5A5D8130EAB
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

#include <assert.h>
#include <map>
#include <memory>
#include <string>
#include "dharma/redirect.h"
#include "dharma/pipe.h"

class Debugger;
class Thread;


class Command
{
public:
    typedef std::map<std::string, std::string> Var;

    explicit Command(const Var& var)
        : var_(var), pending_(false)
    { }

    const std::string& input() const { return input_; }

    const std::string& output();

    void set_input(const std::string& input)
    {
        assert(input_.empty());
        input_ = input;
    }

    void set_expect(const std::string& expect)
    { expectAll_.push_back(expect); }

    void set_expect32(const std::string& expect)
    { expect32_.push_back(expect); }

    void set_expect64(const std::string& expect)
    { expect64_.push_back(expect); }

    void add_echo(const std::string& echo) { echo_ += echo; }

    bool run(std::ostream& log, Debugger*, Thread*);

    bool compare(std::ostream& log, bool exceptionCaught, Thread*);
    bool compare(std::ostream& log,
                 std::vector<std::string>&,
                 bool exceptionCaught);

    bool is_pending() const { return pending_; }

    void assign_to(const std::string& var)
    {
        assignTo_ = var;
    }

    const std::string& assign_to() const { return assignTo_; }

private:
    const Var&  var_;
    Pipe        fifo_;
    std::string input_;     // debugger command
    std::string output_;
    std::string assignTo_;
    std::vector<std::string> expectAll_; // expected output, all systems
    std::vector<std::string> expect32_;  // expected output for 32-bit systems
    std::vector<std::string> expect64_;  // expected output for 64-bit systems

    std::string echo_;
    bool        pending_;
    std::auto_ptr<Redirect> redirect_;
};
#endif // TEST_H__3AE12423_C14E_44CC_8FF1_A5A5D8130EAB
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
