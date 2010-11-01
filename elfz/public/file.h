#ifndef FILE_H__712FDA52_8BB5_4DE2_B361_D064757B1F27
#define FILE_H__712FDA52_8BB5_4DE2_B361_D064757B1F27
//
// $Id: file.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "elfz/public/elf.h"
#include "elfz/public/elfW.h"
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/static_assert.hpp>


namespace ELF
{
    class File
    {
    protected:
        File() : elf_(0), fd_(-1) {}

        explicit File(Elf* elf) : elf_(elf), fd_(-1) {}

        virtual ~File();

    public:
        Elf_Kind kind() const { return elf_kind(elf_); }

        friend bool operator==(const File&, const File&);
        friend class Binary;

        Elf* elf() const { return elf_; }

        virtual size_t readbuf(ElfW(Off), char* buf, size_t) const;

        template<typename T>
        void readval(ElfW(Off) offset, T& val) const
        {
            /* ensure T is POD or built-in */
            //BOOST_STATIC_ASSERT(boost::is_POD<T>::value
            //                 || boost::is_fundamental<T>::value);
            memset(&val, 0, sizeof(T));
            readbuf(offset, reinterpret_cast<char*>(&val), sizeof(T));
        }

        void swap(File& other) throw()
        {
            std::swap(elf_, other.elf_);
            std::swap(fd_, other.fd_);
        }

    protected:

        template<typename T> friend class IterationTraits;

        // Elf* elf() { return elf_; }

        void set_elf(Elf* elf) { assert(elf_ == 0); elf_ = elf; }

        int file_handle() const { return fd_; }

        void set_file_handle(int fd) { fd_ = fd; }

    private:
        Elf* elf_;
        int  fd_;
    };


    bool inline operator==(const File& lhs, const File& rhs)
    {
        return lhs.elf_ == rhs.elf_;
    }


    bool inline operator!=(const File& lhs, const File& rhs)
    {
        return !(lhs == rhs);
    }
} // namespace ELF

// Copyright (c) 2004 Cristian L. Vlasceanu

#endif // FILE_H__712FDA52_8BB5_4DE2_B361_D064757B1F27
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
