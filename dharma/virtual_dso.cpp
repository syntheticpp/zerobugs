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

#include <iostream>
#include "system_error.h"
#include "virtual_dso.h"


VirtualDSOBase::VirtualDSOBase(int fd, loff_t offs, size_t size)
    : buf_(size)
{
    assert(offs >= 0);
    ssize_t n = 0;

    if (lseek64(fd, offs, SEEK_SET) == -1
     || (n = read(fd, &buf_[0], size)) < static_cast<ssize_t>(size))
    {
        SystemError err("virtual dso read");
    #if DEBUG
        std::clog << err.what() << " size=" << n << std::endl;
    #endif
        throw err;
    }
}


VirtualDSOBase::~VirtualDSOBase()
{
}


VirtualDSO::VirtualDSO(int fd, loff_t offs, size_t size)
    : VirtualDSOBase(fd, offs, size)
    , addr_(offs)
    , bin_(elf_memory(image(), size))
{
}


VirtualDSO::VirtualDSO(std::vector<char>& buf, off_t offs)
    : VirtualDSOBase(buf)
    , addr_(offs)
    , bin_(elf_memory(image(), size()))
{
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
