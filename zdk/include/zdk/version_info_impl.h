#ifndef VERSION_INFO_IMPL_H__403A6E77_85FD_4E8D_BFBF_1889351F2FC5
#define VERSION_INFO_IMPL_H__403A6E77_85FD_4E8D_BFBF_1889351F2FC5
//
// $Id: version_info_impl.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/zero.h"

#define DESCRIPTION(s) const char* description() const { return (s); }


template<int MAJOR, int MINOR = 0, int REVISION = 0>
struct VersionInfoImpl : public VersionInfo
{
    const char* copyright() const { return ""; }

    uint32_t version(uint32_t* minor, uint32_t* revision) const
    {
        if (minor)
        {
            *minor = MINOR;
        }
        if (revision)
        {
            *revision = REVISION;
        }
        return MAJOR;
    }
};


#endif // VERSION_INFO_IMPL_H__403A6E77_85FD_4E8D_BFBF_1889351F2FC5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
