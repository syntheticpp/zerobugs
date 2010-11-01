// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: src_tabs.cpp 714 2010-10-17 10:03:52Z root $
//
#include "src_tabs.h"
#include "zdk/shared_string_impl.h"
#include <sstream>

using namespace std;



SourceTabs::~SourceTabs() throw()
{
}


SourceTabs::SourceTabs(const char* name) : Persistent(name)
{
}


void SourceTabs::add_file_name(const RefPtr<SharedString>& fileName)
{
    fileNames_.push_back(fileName);
}


void SourceTabs::on_word(const char*, word_t)
{
}


void SourceTabs::on_string(const char*, const char* str)
{
    add_file_name(shared_string(str));
}


void SourceTabs::on_object_end()
{
}


size_t SourceTabs::write(OutputStream* stream) const
{
    size_t bytesWritten = 0;

    container_type::const_iterator i = fileNames_.begin();
    for (size_t n = 0; i != fileNames_.end(); ++i, ++n)
    {
        ostringstream name;

        name << "tab_" << n;
        bytesWritten += stream->write_string(name.str().c_str(), (*i)->c_str());
    }
    // bytesWritten += stream->write_word("size", n);
    return bytesWritten;
}
