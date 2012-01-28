#ifndef LDSOCONF_H__4534B6E5_24B3_45A1_8F8B_2136D88579C7
#define LDSOCONF_H__4534B6E5_24B3_45A1_8F8B_2136D88579C7
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
//
#include <string>
#include <vector>

void ldsoconf_parse(const char* filename,
                    std::vector<std::string>& dirs // out
                   );
#endif // LDSOCONF_H__4534B6E5_24B3_45A1_8F8B_2136D88579C7
