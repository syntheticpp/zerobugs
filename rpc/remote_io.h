#ifndef REMOTE_IO_H__8F3FD7C5_080C_4E2D_9CB9_6F874EBA8FF4
#define REMOTE_IO_H__8F3FD7C5_080C_4E2D_9CB9_6F874EBA8FF4
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
//
namespace RPC
{
    enum IOCommand
    {
        RIO_NONE,
        RIO_OPEN,
        RIO_READ,
        RIO_READ_LINK,
        RIO_SEEK,
        RIO_CLOSE,
    };
}
#endif // REMOTE_IO_H__8F3FD7C5_080C_4E2D_9CB9_6F874EBA8FF4
