//
// $Id: thread_db_error.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/format.hpp>
#include "dharma/canonical_path.h"
#include "thread_db_error.h"


const char* thread_db_error::what() const throw()
{
    if (what_.empty()) try
    {
        boost::format fmt("%s:%d: %s");
        const char* msg = "Unknown thread_db error";

        switch (err_)
        {
        case TD_OK: msg = "No error"; break;
        case TD_ERR: msg = "No further specified error"; break;
        case TD_NOTHR: msg = "No matching thread found"; break;
        case TD_NOSV: msg = "No matching synchronization handle found"; break;
        case TD_NOLWP: msg = "No matching light-weighted process found"; break;
        case TD_BADPH: msg = "Invalid process handle"; break;
        case TD_BADTH: msg = "Invalid thread handle"; break;
        case TD_BADSH: msg = "Invalid synchronization handle"; break;
        case TD_BADTA: msg = "Invalid thread agent"; break;
        case TD_BADKEY: msg = "Invalid key"; break;
        case TD_NOMSG: msg = "No event available"; break;
        case TD_NOFPREGS: msg = "No floating-point register content available"; break;
        case TD_NOLIBTHREAD: msg = "Application not linked with thread library"; break;
        case TD_NOEVENT: msg = "Requested event is not supported"; break;
        case TD_NOCAPAB: msg = "Capability not available"; break;
        case TD_DBERR: msg = "Internal debug library error"; break;
        case TD_NOAPLIC: msg = "Operation is not applicable"; break;
        case TD_NOTSD: msg = "No thread-specific data available"; break;
        case TD_MALLOC: msg = "Out of memory"; break;
        case TD_PARTIALREG: msg = "Not entire register set was read or written"; break;
        case TD_NOXREGS: msg = "X register set not available for given thread"; break;
    #if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 3)
        case TD_NOTALLOC: msg = "Thread has not yet allocated TLS for given module"; break;
        case TD_VERSION: msg = "Version if libpthread and libthread_db do not match"; break;

     // case TD_NOTLS: msg = "There is no TLS segment in the given module";
    #endif
        }
        what_ = (fmt % canonical_path(file_.c_str()) % line_ % msg).str();
    }
    catch (...)
    {
        return "unknown thread_db error";
    }
    return what_.c_str();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
