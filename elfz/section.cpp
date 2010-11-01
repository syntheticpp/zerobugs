//
// $Id: section.cpp 711 2010-10-16 07:09:23Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <stdexcept>
#include "public/file.h"
#include "public/error.h"
#include "public/section.h"


using namespace ELF;
using namespace std;


Section::Section(Elf* elf, Elf_Scn* scn)
    : scn_(scn)
    , hdr_(elf, scn)
    , data_(0)
{
}


boost::shared_ptr<Section> Section::next() const
{
    boost::shared_ptr<Section> nextScn;

    if (Elf_Scn* scn = elf_nextscn(const_cast<Elf*>(hdr_.elf()), scn_))
    {
        boost::shared_ptr<Section> p(new Section(hdr_.elf(), scn));
        nextScn = p;
    }
    return nextScn;
}


const Elf_Data& Section::data() const
{
    assert(scn_);

    if (!data_)
    {
        data_ = elf_getdata(scn_, 0);
        if (!data_)
        {
            assert(header().name());

            string name(header().name());
            throw Error((name + ": null data").c_str());
        }
        // fixme: support data list
        assert(!elf_getdata(scn_, data_));
    }
    return *data_;
}


boost::shared_ptr<Section> IterationTraits<Section>::first(File& file)
{
    boost::shared_ptr<Section> firstScn;

    if (Elf_Scn* scn = elf_getscn(file.elf(), 0))
    {
        boost::shared_ptr<Section> p(new Section(file.elf(), scn));
        firstScn = p;
    }
    return firstScn;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
