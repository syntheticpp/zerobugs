#ifndef AUTO_FILE_H__1028357775
#define AUTO_FILE_H__1028357775
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
#include <cstdio>
#if HAVE_UNISTD_H
 #include <unistd.h>
#else
 #include <io.h>
 #define close _close
#endif
#include "auto_handle.h"

typedef auto_handle<FILE*> auto_file;

template<>
struct handle_traits<FILE*>
{
    static FILE* null_value()
    { return 0; }

    static void dispose(FILE* f) throw()
    { if (f) fclose(f); }
};



struct ZDK_LOCAL fd_traits
{
    static int null_value()
    {
        return -1;
    }

    static void dispose(int fd) throw()
    {
        if (fd != -1)
        {
            close(fd);
        }
    }
};

typedef auto_handle<int, fd_traits> auto_fd;

#endif // AUTO_FILE_H__1028357775
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
