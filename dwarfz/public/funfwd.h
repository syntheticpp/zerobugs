#ifndef FUNFWD_H__E83AA504_E791_424F_8D78_4EDF85AD2095
#define FUNFWD_H__E83AA504_E791_424F_8D78_4EDF85AD2095
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
#include <vector>

namespace Dwarf
{
    class Function;
    class MemFun;
    class Type;

    typedef std::vector<std::shared_ptr<Type> > TypeList;

    typedef std::vector<std::shared_ptr<Function> > FunList;
    typedef std::vector<std::shared_ptr<MemFun> > MethodList;
}
#endif // FUNFWD_H__E83AA504_E791_424F_8D78_4EDF85AD2095
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
