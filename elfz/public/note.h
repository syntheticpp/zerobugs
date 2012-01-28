#ifndef NOTE_H__A294DB09_F457_4EE5_991D_81CAEDEE3D6E
#define NOTE_H__A294DB09_F457_4EE5_991D_81CAEDEE3D6E
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
#include <vector>
#include "elfz/public/selector.h"


namespace ELF
{
    class File;

    /**
     * C++ wrapper for accessing Elf notes.
     */
    class Note : public Selector
    {
    public:
        Note(const File&, const unsigned char*);

        /// @return the name size
        ElfW(Word) namesz() const;

        /// @return the data size
        ElfW(Word) descsz() const;

        ElfW(Word) type() const;

        const char* name() const;

        bool is_null() const { return note32_ == 0; }

        size_t total_rounded_size() const
        {
            return totalRoundedSize_;
        }

        const std::vector<unsigned char>& data() const
        {
            return data_;
        }

        const void* d_ptr() const { return &data_[0]; }

    private:
        union
        {
            Elf32_Nhdr* note32_;
            Elf64_Nhdr* note64_;
        };

        size_t totalRoundedSize_;

        std::string name_;
        std::vector<unsigned char> data_;
    };
}
#endif // NOTE_H__A294DB09_F457_4EE5_991D_81CAEDEE3D6E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
