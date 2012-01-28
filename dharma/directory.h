#ifndef DIRECTORY_H__854AF40A_EB89_4A5B_B655_557086D635B4
#define DIRECTORY_H__854AF40A_EB89_4A5B_B655_557086D635B4
//
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
#include <string>
#include <vector>
#include "zdk/export.h"

/**
 * Abstracts out the OS-level details for iterating
 * through a directory.
 */
class ZDK_LOCAL Directory
{
public:
    typedef std::vector<std::string> container_type;
    typedef container_type::size_type size_type;
    typedef container_type::value_type value_type;

    class Iterator
    {
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef container_type::value_type value_type;
        typedef size_type difference_type;
        typedef value_type::pointer pointer;
        typedef value_type::reference reference;

        Iterator& operator++();
        Iterator& operator--();
        // NOTE: postfix forms intentionally not supported

        std::string operator*() const;

        bool operator==(const Iterator&) const;

        bool operator!=(const Iterator& that) const
        { return !(*this==(that)); }

        const std::string& short_path() const { return *iter_; }

    private:
        friend class Directory; // only Directory can construct
        Iterator(container_type::const_iterator, const std::string&);

        container_type::const_iterator iter_;
        const std::string& path_;
    }; // Iterator;

    typedef Iterator const_iterator;

    // NOTE: at this time we don't need non-const iterator, reverse_iterator

public:
    explicit Directory(
        const std::string& path,
        const char* pattern = 0,
        bool recursive = false);

    const_iterator begin() const;
    const_iterator end() const;

    size_type size() const { return files_.size(); }
    bool empty() const { return files_.empty(); }

private:
    std::string path_;
    container_type files_;
};

#endif // DIRECTORY_H__854AF40A_EB89_4A5B_B655_557086D635B4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
