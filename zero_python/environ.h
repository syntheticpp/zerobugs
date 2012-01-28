#ifndef ENVIRON_H__03028333_C72A_4721_AFEA_4F8C06A789A7
#define ENVIRON_H__03028333_C72A_4721_AFEA_4F8C06A789A7
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

#include <map>
#include <string>
#include "dharma/sarray.h"


void env_to_map(const char* const*, std::map<std::string, std::string>&);


void map_to_env(const std::map<std::string, std::string>&, SArray&);


#endif // ENVIRON_H__03028333_C72A_4721_AFEA_4F8C06A789A7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
