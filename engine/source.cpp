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

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include "generic/state_saver.h"
#include "source.h"
#include "readline/terminal.h"


using namespace std;


SourceListing::SourceListing(const string& name)
    : name_(name), currentLine_(0), symbolLine_(0)
{
    /* if your source code has lines that exceed 4k, tough luck */
    vector<char> buf(4096);

    ifstream fin(name.c_str());

    while (fin.getline(&buf[0], buf.size()))
    {
        lines_.push_back(&buf[0]);
    }
}


SourceListing::~SourceListing()
{
}


void SourceListing::set_current_line(size_t line)
{
    if (line >= lines_.size() + 1)
    {
        line = lines_.empty() ? 1 : lines_.size();
    }
    currentLine_ = line;
}


size_t SourceListing::list(ostream& outs, size_t howMany) const
{
    StateSaver<ios, ios::fmtflags> saveState(cout);

    size_t i = currentLine_ ? currentLine_ - 1 : 0;

    if (i == 0)
    {
        outs << name_ << endl;
    }
    /* show some context around the current symbol */
    if ((i + 1) == symbol_line())
    {
        if (i > 5)
        {
            i -= 5;
            howMany += 5;
        }
        else
        {
            howMany += i;
            i = 0;
        }
    }

    const size_t last = i + min(howMany, lines_.size() - i);

    outs << setfill(' ');
    for (; i != last; ++i)
    {
        const size_t ln = i + 1;
        outs << setw(6) << ln;

        if (ln == symbol_line())
        {
            outs << "==>";
        }
        else
        {
            outs << "   ";
        }
        outs << lines_[i] << endl;
    }
    return last;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
