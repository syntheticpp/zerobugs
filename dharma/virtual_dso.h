#ifndef VIRTUAL_DSO_H__DA3B9C74_CDA0_4DD4_8747_7B7A646A58DF
#define VIRTUAL_DSO_H__DA3B9C74_CDA0_4DD4_8747_7B7A646A58DF
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

#include <vector>
#include "generic/auto_file.h"
#include "elfz/public/binary.h"


/**
 * Helper base, for constructing the image in memory
 * of a mapped virtual dynamic shared object (the
 * image is passed as an argument to elf_memory in the
 * derived class).
 * @see http://kerneltrap.org/node/3405
 * @see http://moonbase.rydia.net/mental/blog/linux/linux-gate-so-1.html
 */
class ZDK_LOCAL VirtualDSOBase
{
protected:
    /**
     * construct the image from a file
     */
    VirtualDSOBase(int fd, loff_t offs, size_t length);

    /**
     * construct the image from a buffer
     */
    explicit VirtualDSOBase(std::vector<char>& buf)
    { buf_.swap(buf); }

    virtual ~VirtualDSOBase();

    char* image() { return &buf_[0]; }

private:
    std::vector<char> buf_;

public:
    size_t size() const { return buf_.size(); }
};


/**
 * Linux-specific, for handling virtual dynamic shared
 * objects mapped into memory
 */
class ZDK_LOCAL VirtualDSO : public VirtualDSOBase
{
public:
    VirtualDSO(int fd, loff_t offs, size_t length);

    VirtualDSO(std::vector<char>&, off_t);

    ELF::Binary& binary() { return bin_; }

    off_t addr() const { return addr_; }

private:
    off_t addr_;
    ELF::Binary bin_;
};


#endif // VIRTUAL_DSO_H__DA3B9C74_CDA0_4DD4_8747_7B7A646A58DF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
