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

#include "zdk/config.h"
#include <string.h>
#include <iostream>
#ifndef _GNU_SOURCE
 #define _GNU_SOURCE
#endif
#include <cassert>
#include <map>
#include <sstream>
#include <stdexcept>
#include "sigutil.h"

using namespace std;


////////////////////////////////////////////////////////////////
static const char* names[] =
{
        "SIGHUP",
        "SIGINT",
        "SIGQUIT",
        "SIGILL",
        "SIGTRAP",
        "SIGABRT",
        "SIGIOT",
        "SIGBUS",
        "SIGFPE",
        "SIGKILL",
        "SIGUSR1",
        "SIGSEGV",
        "SIGUSR2",
        "SIGPIPE",
        "SIGALRM",
        "SIGTERM",
        "SIGSTKFLT",
        "SIGCHLD",
        "SIGCONT",
        "SIGSTOP",
        "SIGTSTP",
        "SIGTTIN",
        "SIGTTOU",
        "SIGURG",
        "SIGXCPU",
        "SIGXFSZ",
        "SIGVTALRM",
        "SIGPROF",
        "SIGWINCH",
        "SIGIO",
        "SIGPOLL",
        "SIGPWR",
        "SIGSYS",
        0
};

////////////////////////////////////////////////////////////////
const char* const* sig_name_list()
{
    return names;
}

////////////////////////////////////////////////////////////////
static map<string, int>& signal_map()
{
    static map<string, int> theMap;
    if (theMap.empty())
    {
        theMap["SIGHUP"] = SIGHUP;
        theMap["SIGINT"] = SIGINT;
        theMap["SIGQUIT"] = SIGQUIT;
        theMap["SIGILL"] = SIGILL;
        theMap["SIGTRAP"] = SIGTRAP;
        theMap["SIGABRT"] = SIGABRT;
        theMap["SIGIOT"] = SIGIOT;
        theMap["SIGBUS"] = SIGBUS;
        theMap["SIGFPE"] = SIGFPE;
        theMap["SIGKILL"] = SIGKILL;
        theMap["SIGUSR1"] = SIGUSR1;
        theMap["SIGSEGV"] = SIGSEGV;
        theMap["SIGUSR2"] = SIGUSR2;
        theMap["SIGPIPE"] = SIGPIPE;
        theMap["SIGALRM"] = SIGALRM;
        theMap["SIGTERM"] = SIGTERM;
#if HAVE_SIGSTKFLT
        theMap["SIGSTKFLT"] = SIGSTKFLT;
#endif
        theMap["SIGCHLD"] = SIGCHLD;
        theMap["SIGCONT"] = SIGCONT;
        theMap["SIGSTOP"] = SIGSTOP;
        theMap["SIGTSTP"] = SIGTSTP;
        theMap["SIGTTIN"] = SIGTTIN;
        theMap["SIGTTOU"] = SIGTTOU;
        theMap["SIGURG"] = SIGURG;
        theMap["SIGXCPU"] = SIGXCPU;
        theMap["SIGXFSZ"] = SIGXFSZ;
        theMap["SIGVTALRM"] = SIGVTALRM;
        theMap["SIGPROF"] = SIGPROF;
        theMap["SIGWINCH"] = SIGWINCH;
        theMap["SIGIO"] = SIGIO;
#if HAVE_SIGPOLL
        theMap["SIGPOLL"] = SIGPOLL;
#endif
#if HAVE_SIGPWR
        theMap["SIGPWR"] = SIGPWR;
#endif
        theMap["SIGSYS"] = SIGSYS;
    }

    return theMap;
}


////////////////////////////////////////////////////////////////
int sig_from_name(const string& name)
{
    int result = 0;
    map<string, int>::const_iterator i = signal_map().find(name);
    if (i != signal_map().end())
    {
        return i->second;
    }
    else
    {
        return strtol(name.c_str(), 0, 0);
    }
    return result;
}


////////////////////////////////////////////////////////////////
string sig_name(int sig)
{
    map<string, int>::const_iterator i = signal_map().begin();
    map<string, int>::const_iterator e = signal_map().end();

    for (; i != e; ++i)
    {
        if (i->second == sig)
        {
            return i->first;
        }
    }
    ostringstream os;
    os << "SIG" << sig;

    return os.str();
}


////////////////////////////////////////////////////////////////
string sig_description(int sig)
{
    const char* str = strsignal(sig);
    assert(str);

    ostringstream out;

    out << str << " (signal " << sig << ")";

    return out.str();
}


static void ignore(int signum, siginfo_t*, void* uc)
{
#if DEBUG
    clog << __func__ << ": " << sig_name(signum) << endl;
#endif
}


////////////////////////////////////////////////////////////////
IgnoreSignalInScope::IgnoreSignalInScope(int sig) : sig_(sig)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof (struct sigaction));
    memset(&sa_, 0, sizeof (struct sigaction));

    //sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = ignore;

    ::sigaction(sig, &sa, &sa_);
}


////////////////////////////////////////////////////////////////
IgnoreSignalInScope::~IgnoreSignalInScope() throw()
{
    ::sigaction(sig_, &sa_, 0);
}


////////////////////////////////////////////////////////////////
BlockSignalsInScope::BlockSignalsInScope(const sigset_t& newMask)
{
    ::sigprocmask(SIG_BLOCK, &newMask, &mask_);
}


////////////////////////////////////////////////////////////////
BlockSignalsInScope::BlockSignalsInScope(int signum)
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, signum);

    ::sigprocmask(SIG_BLOCK, &mask, &mask_);
}


////////////////////////////////////////////////////////////////
BlockSignalsInScope::~BlockSignalsInScope()
{
    ::sigprocmask(SIG_SETMASK, &mask_, 0);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
