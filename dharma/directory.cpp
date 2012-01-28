//
// $Id$
//
// Linux implementation for the Directory class
//
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <errno.h>
#include <sys/stat.h>

// unix-specific:
#include <dirent.h>
#include <fnmatch.h>

// C++ includes:
#include <algorithm>
#include <functional>
#include <stdexcept>

#include <boost/utility.hpp>
#include <boost/tokenizer.hpp>

// local headers
#include "directory.h"
#include "system_error.h"
#include "zdk/export.h"


/**
 * A helper class, so that we can build a vector of
 * strings (representing entries in a directory) in
 * an exception-safe manner.
 */
class ZDK_LOCAL DirList : boost::noncopyable
{
public:
    explicit DirList(const char* path)
        : size_(0), namelist_(0)
    {
        assert(path);

        size_ = scandir(path, &namelist_, 0, 0);
        if (size_ < 0)
        {
            throw SystemError(path);
        }
    }

    ~DirList() throw()
    {
        for (int i(0); i < size_; ++i)
        {
            free(namelist_[i]);
        }
        free(namelist_);
    }

    int size() const { return size_; }

    const char* operator[](int i) const
    { return i < size_ ? namelist_[i]->d_name : 0; }

private:
    int size_;
    struct dirent** namelist_;
};


class ZDK_LOCAL FileNameFilter : public std::unary_function<std::string, bool>
{
    typedef boost::char_separator<char> Separator;
    typedef boost::tokenizer<Separator> Tokenizer;

public:
    explicit FileNameFilter(const char* pattern)
        : pattern_(pattern ? pattern : "")
        , tok_(pattern_, Separator(" "))
    {
    }

    bool operator()(const std::string& name) const
    {
        // return fnmatch(pattern_.c_str(), name.c_str(), 0) == 0;

        if (pattern_.empty())
        {
            return true;
        }

        Tokenizer::const_iterator i(tok_.begin());
        for (; i != tok_.end(); ++i)
        {
            if (fnmatch((*i).c_str(), name.c_str(), 0) == 0)
            {
                return true;
            }
        }
        return false;
    }

private:
    std::string pattern_;
    Tokenizer tok_;
};


////////////////////////////////////////////////////////////////
Directory::Iterator::Iterator(
    container_type::const_iterator iter,
    const std::string& path)
: iter_(iter), path_(path)
{
}


////////////////////////////////////////////////////////////////
Directory::Iterator& Directory::Iterator::operator++()
{
    ++iter_;
    return *this;
}


////////////////////////////////////////////////////////////////
Directory::Iterator& Directory::Iterator::operator--()
{
    --iter_;
    return *this;
}


////////////////////////////////////////////////////////////////
std::string Directory::Iterator::operator*() const
{
    std::string result;

    if (path_ != ".")
    {
        result = path_ + "/";
    }
    result += *iter_;
    return result;
}

////////////////////////////////////////////////////////////////
bool Directory::Iterator::operator==(
    const Directory::Iterator& other) const
{
    return (iter_ == other.iter_);
}

////////////////////////////////////////////////////////////////
Directory::Directory
(
    const std::string& path,
    const char* pattern,
    bool recursive
)
  : path_(path)
{
    DirList dirList(path_.c_str());

    files_.reserve(dirList.size());
    for (int i(0); i < dirList.size(); ++i)
    {
        files_.push_back(dirList[i]);
    }

// <recursive>
    struct stat statbuf;
    std::vector<std::string> tmp;
// </recursive>

    FileNameFilter filt(pattern);

    std::vector<std::string>::iterator i = files_.begin();
    std::vector<std::string>::iterator j = files_.begin();

    for (; j != files_.end(); ++j)
    {
        if (filt(*j))
        {
            *i++ = *j;
        }

        if (!recursive || *j == "." || *j == "..")
        {
            continue;
        }

        const std::string fname(path_ + "/" + *j);

        while (::stat(fname.c_str(), &statbuf) < 0)
        {
            if (errno != EINTR)
            {
                throw SystemError("stat(" + fname + ")");
            }
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            Directory dir(fname.c_str(), pattern, true);
            tmp.insert(tmp.end(), dir.begin(), dir.end());
        }
    }

    files_.erase(i, files_.end());
    files_.insert(files_.end(), tmp.begin(), tmp.end());
}

////////////////////////////////////////////////////////////////
Directory::Iterator Directory::begin() const
{
    return Iterator(files_.begin(), path_);
}

////////////////////////////////////////////////////////////////
Directory::Iterator Directory::end() const
{
    return Iterator(files_.end(), path_);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
