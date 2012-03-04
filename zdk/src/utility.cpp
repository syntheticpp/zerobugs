//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/utility.h"

#include <iomanip>
#include <iostream>

std::ostream&
operator<<(std::ostream& os, const Frame& frame)
{
    StateSaver<std::ios, std::ios::fmtflags> save(os);
    os << "Frame #" << frame.index() << ":";

    os << std::hex << std::showbase;
    os << " pc=" << frame.program_count();
    os << " sp=" << frame.stack_pointer();
    os << " fp=" << frame.frame_pointer();

    return os;
}

