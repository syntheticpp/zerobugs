#ifndef SIGUTIL_H__1055661665
#define SIGUTIL_H__1055661665
//
// $Id: sigutil.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <signal.h>
#include <string>
#include <boost/utility.hpp>
#include "zdk/export.h"

/**
 * Converts signal number to human-readble description
 */
ZDK_LOCAL std::string sig_description(int);

ZDK_LOCAL int sig_from_name(const std::string&);

ZDK_LOCAL std::string sig_name(int);

ZDK_LOCAL const char* const* sig_name_list();


/**
 * Uses resource-acquisition-is-initialization for
 * ignoring a signal within a given scope
 */
CLASS IgnoreSignalInScope : private ::boost::noncopyable
{
public:
    explicit IgnoreSignalInScope(int sig);

    ~IgnoreSignalInScope() throw();

private:
    int sig_;
    struct sigaction sa_;
};


/**
 * Uses resource-acquisition-is-initialization for
 * changing/auto-restoring the mask of blocked signals
 */
CLASS BlockSignalsInScope : private ::boost::noncopyable
{
public:
    explicit BlockSignalsInScope(const sigset_t&);
    explicit BlockSignalsInScope(int);
    ~BlockSignalsInScope();

    operator const sigset_t*() const { return &mask_; }

private:
    sigset_t mask_;
};

#endif // SIGUTIL_H__1055661665
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
