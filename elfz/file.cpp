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

#include <errno.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <unistd.h>
#include "public/file.h"


using namespace std;
using namespace ELF;


File::~File()
{
    // we do not own the fd_, so do not close it

    if (elf_)
    {
        elf_end(elf_);
    }
    elf_ = 0;
}


size_t File::readbuf(ElfW(Off) offs, char* buf, size_t len) const
{
    assert(file_handle() >= 0);

    for (;;)
    {
        // memorize current position in file
        const off_t pos = lseek(file_handle(), 0, SEEK_CUR);
        if (pos < 0)
        {
            if (errno == EINTR) continue;
            throw runtime_error(strerror(errno));
        }
        if (lseek(file_handle(), offs, SEEK_SET) < 0)
        {
            if (errno == EINTR) continue;
            throw runtime_error(strerror(errno));
        }
        ssize_t count = ::read(file_handle(), buf, len);
        if (count == 0)
        {
            return 0;
        }
        // restore file position
        if (lseek(file_handle(), pos, SEEK_SET) < 0)
        {
            if (errno == EINTR) continue;
            throw runtime_error(strerror(errno));
        }

        // any return count (including -1) different from
        // the length we specified is not acceptable
        if (static_cast<ElfW(Xword)>(count) != len)
        {
            if (errno == EINTR) continue;
        #if DEBUG
            clog << "read " << count << ", expected " << len << endl;
        #endif
            throw runtime_error(strerror(errno));
        }
        break;
    }
    return len;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
