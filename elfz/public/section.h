#ifndef SECTION_H__B4C7F7DA_7D51_477B_84D6_BE46064515DE
#define SECTION_H__B4C7F7DA_7D51_477B_84D6_BE46064515DE
//
// $Id: section.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "elfz/public/headers.h"
#include "elfz/public/iterator.h"

namespace ELF
{
    /**
     * Wrapper class for elf sections
     */
    class Section
    {
    public:
        typedef SectionHdr Header;


        Section(Elf*, Elf_Scn*);

        bool is_null() const { return scn_ == 0; }

        /**
         * Return the section header, which contains
         * information such as the section's type, name
         * and so on.
         */
        const Header& header() const { return hdr_; }

        /**
         * Get the next ELF section; if last section,
         * return a section for which is_null() is true.
         */
        boost::shared_ptr<Section> next() const;

        const Elf_Data& data() const;

        friend bool operator==(const Section&, const Section&);

    protected:
        Elf_Scn* scn() const { return scn_; }

    private:
        Elf_Scn*            scn_;
        const SectionHdr    hdr_;
        mutable Elf_Data*   data_;
    };


    inline bool operator==(const Section& lhs, const Section& rhs)
    {
        return lhs.scn_ == rhs.scn_;
    }

    inline bool operator!=(const Section& lhs, const Section& rhs)
    {
        return !(lhs == rhs);
    }


    inline bool is_symbol_table(const Section& sec)
    {
        return (sec.header().type() == SHT_SYMTAB
            ||  sec.header().type() == SHT_DYNSYM);
    }


    inline bool is_dynamic_symbol_table(const Section& sec)
    {
        return sec.header().type() == SHT_DYNSYM;
    }


    inline const char* section_name(const Section& sec)
    {
        return sec.header().name();
    }


    template<> struct IterationTraits<Section>
    {
        static boost::shared_ptr<Section> first(File&);

        static boost::shared_ptr<Section> next(File*, const Section& sec)
        {
            return sec.next();
        }
    };
} // namespace ELF

#endif // SECTION_H__B4C7F7DA_7D51_477B_84D6_BE46064515DE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
