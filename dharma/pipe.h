#ifndef PIPE_H__57ACA83E_192A_474F_82D6_5973976719E1
#define PIPE_H__57ACA83E_192A_474F_82D6_5973976719E1
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

#include <new>
#include <boost/utility.hpp>
#include <string>
#include "zdk/export.h"


class ZDK_LOCAL Pipe : boost::noncopyable
{
public:
    explicit Pipe(bool nonblock = true);

    ~Pipe() throw();

    bool write(const void*, size_t, bool no_throw = false) volatile;

    bool read(void*, size_t, bool no_throw = false);

    template<typename T>
    void write(const T& t) volatile
    {
        write(&t, sizeof(t));
    }
    template<typename T>
    void read(T& t)
    {
        read(&t, sizeof(t));
    }
    template<typename T>
    bool write(const T& t, std::nothrow_t) volatile
    {
        return write(&t, sizeof(t), true);
    }
    template<typename T>
    bool read(T& t, std::nothrow_t)
    {
        return read(&t, sizeof(t), true);
    }
    void write(const std::string& s)
    {
        write(s.c_str(), s.length());
    }

    int output() const { return pipe_[0]; }

    int input() const { return pipe_[1]; }

    void close_output();

    void close_input();

protected:
    void error(const char*, bool no_throw = false) volatile;

private:
    Pipe(const Pipe&);
    Pipe& operator=(const Pipe&);

    int pipe_[2];
};
#endif // PIPE_H__57ACA83E_192A_474F_82D6_5973976719E1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
