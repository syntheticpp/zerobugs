#ifndef FUNFWD_H__E83AA504_E791_424F_8D78_4EDF85AD2095
#define FUNFWD_H__E83AA504_E791_424F_8D78_4EDF85AD2095
//
// $Id: funfwd.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/shared_ptr.hpp>
#include <vector>

namespace Dwarf
{
    class Function;
    class MemFun;
    class Type;

    typedef std::vector<boost::shared_ptr<Type> > TypeList;

    typedef std::vector<boost::shared_ptr<Function> > FunList;
    typedef std::vector<boost::shared_ptr<MemFun> > MethodList;
}
#endif // FUNFWD_H__E83AA504_E791_424F_8D78_4EDF85AD2095
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
