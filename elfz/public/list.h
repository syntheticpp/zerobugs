#ifndef LIST_H__E1A00829_8B18_424E_B092_B4280047173C
#define LIST_H__E1A00829_8B18_424E_B092_B4280047173C
//
// $Id: list.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "elfz/public/file.h"
#include "elfz/public/iterator.h"
#include <algorithm>


namespace ELF
{
    /**
     * Pseudo-container for traversing ELF sections,
     * symbol tables, archives, etc.
     */
    template<typename T>
    class List
    {
        File* file_;

    public:
        typedef Iterator<T, T*, T&> iterator;
        typedef Iterator<T, const T*, const T&> const_iterator;

        template<typename U>
        explicit List(U& file) : file_(&file) { }

     /*
        template<typename U>
        explicit List(const List<U>& other) : file_(other.file_)
        { }

        template<typename U>
        operator List<U>& () { return *this; }

        template<typename U>
        operator const List<U>& () const { return *this; }
      */

        void swap(List& other) throw()
        {
            std::swap(file_, other.file_);
        }

        const_iterator begin() const
        { return file_ ? const_iterator(*file_) : const_iterator(); }

        const_iterator end() const
        { return const_iterator(); }

        iterator begin()
        { return file_ ? iterator(*file_) : iterator(); }

        iterator end() { return iterator(); }
    };
}
#endif // LIST_H__E1A00829_8B18_424E_B092_B4280047173C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
