//
// $Id: template_param.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "public/template_param.h"


TemplateTypeParamImpl::TemplateTypeParamImpl (
    SharedString* name,
    DataType* type
)
: name_(name)
, type_(type)
{
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
