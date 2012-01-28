#ifndef REDIRECT_H__4DD6FE82_ECB1_4A01_B8A2_B414BDBE75E3
#define REDIRECT_H__4DD6FE82_ECB1_4A01_B8A2_B414BDBE75E3
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

#include "generic/auto_file.h"
#include "generic/export.h"
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>


class ZDK_LOCAL Redirect : boost::noncopyable
{
public:
    class Undo
    {
        int fd_;
        auto_fd dup_;

    public:
        explicit Undo(int fd);
        ~Undo();
    };

    Redirect(int fd, auto_fd&);

    Redirect(int fd, int toFile, bool autoClose = false);

    virtual ~Redirect();

private:
    void init(int fd, int toFile);

    int dest_; // redirect destination, or -1 if autoClose is false
    Undo* undo_;
};



#endif // REDIRECT_H__4DD6FE82_ECB1_4A01_B8A2_B414BDBE75E3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
