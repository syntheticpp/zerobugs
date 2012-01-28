#ifndef BINARY_H__D6B3BDBE_546B_422F_B7CA_DB38397DB70A
#define BINARY_H__D6B3BDBE_546B_422F_B7CA_DB38397DB70A
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

#include <memory>
#include <string>
#include <boost/shared_ptr.hpp>
#include "zdk/config.h"
#include "elfz/public/list.h"


namespace ELF
{
    class ElfHdr;
    class ProgramHeader;
    class Section;
    class SymbolTable;


    /**
     * Wrapper class for Elf descriptors.
     * @note Binaries exclusively own the file handle,
     * therefore they are noncopyable;it is up to the client
     * code to wrap Binary instances into reference-counted
     * smart pointers.
     */
    class Binary
        : public File
        , private List<Binary>
        , private List<ProgramHeader>
        , private List<Section>
        , private List<SymbolTable>
    {
        Binary(const Binary&);
        Binary& operator=(const Binary&);

    public:
        typedef ElfHdr Header;

        /**
         * Construct a read-only ELF::Binary for the given
         * file (file has to exist on disk).
         * @throw std::runtime_error if the file cannot
         * be open for reading.
         */
        explicit Binary(const char* filename);
        explicit Binary(Elf*);

        virtual ~Binary();

        /**
         * Verifies that the associated file is indeed an ELF
         * @todo: this is redundant with kind() == ELF_K_ELF
         * and will be deprecated.
         */
        bool check_format() const;

        Header header() const;
        uint8_t abi() const;
        uint8_t abi_version() const;

        const std::string& name() const { return filename_; }

        virtual const std::string& executable_name() const
        { return filename_; }

        /**
         * @return a pseudo-container of Section's
         */
        List<Section>& sections() { return *this; }

        const List<Section>& sections() const { return *this; }

        /**
         * @return a pseudo-container of symbol tables
         */
        List<SymbolTable>& symbol_tables() { return *this; }

        const List<SymbolTable>& symbol_tables() const { return *this; }

        /**
         * @return a pseudo-list of descriptors,
         * for iterating over objects in Elf archives
         */
        List<Binary>& archive() { return *this; }

        const List<Binary>& archive() const { return *this; }

        /**
         * @return a pseudo-list of program headers
         */
        List<ProgramHeader>& program_headers() { return *this; }

        const List<ProgramHeader>& program_headers() const { return *this; }

        /**
         * Get archive header
         */
        Elf_Arhdr* arhdr() const
        {
            return elf_getarhdr(const_cast<Elf*>(elf()));
        }

        boost::shared_ptr<Binary> next(File* arch) const;

        ElfW(Addr) get_load_addr(ElfW(Xword)* memsz = 0) const;

        ElfW(Addr) get_plt_addr(size_t* size = 0) const;

        bool is_null() const;

        const std::string& dynamic_linker() const;

    private:
        std::string         filename_;
        mutable ElfW(Addr)  pltAddr_;
        mutable size_t      pltSize_;
        mutable std::string interp_;
        mutable uint8_t     abi_;
        mutable uint8_t     abiVersion_;
    };


    template<> struct IterationTraits<Binary>
    {
        static boost::shared_ptr<Binary> first(File&);

        static boost::shared_ptr<Binary> next(File*, const Binary&);
    };


    ElfW(Addr) get_linker_debug_struct_addr(const Binary&);

    std::string debug_link(const Binary&, size_t* crc = NULL);

} // namespace ELF

// Copyright (c) 2004, 2006, 2007 Cristian L. Vlasceanu

#endif // BINARY_H__D6B3BDBE_546B_422F_B7CA_DB38397DB70A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
