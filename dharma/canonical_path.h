#ifndef CANONICAL_PATH_H__386CB99A_F005_4AE9_9530_93BEB7D44C21
#define CANONICAL_PATH_H__386CB99A_F005_4AE9_9530_93BEB7D44C21
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

#include <string>
#include "zdk/export.h"
#include "zdk/shared_string.h"
#include "zdk/ref_ptr.h"


std::string canonical_path(const char*);

/**
 * @note CALLER MUST free() RESULT
 */
char* abspath(const char*);

RefPtr<SharedString> abspath(const RefPtr<SharedString>&);

inline char* abspath(const std::string& path)
{
    return abspath(path.c_str());
}



class ZDK_LOCAL CanonicalPath
{
    explicit CanonicalPath(const char*);

public:
    explicit CanonicalPath(const std::string& path) : path_(path) {}

    const std::string& get() const
    {
        if (!buf_.empty() || path_[0] != '/' || path_.find("./") != path_.npos)
        {
            return buf_ = canonical_path(path_.c_str());
        }
        return path_;
    }

    operator const std::string&() { return get(); }

    const char* c_str() const { return get().c_str(); }

private:
    const std::string& path_;
    mutable std::string buf_;
};


#endif // CANONICAL_PATH_H__386CB99A_F005_4AE9_9530_93BEB7D44C21
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
