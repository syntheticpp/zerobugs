//
// $Id: object.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "public/object.h"


size_t Stab::Object::instances_ = 0;


Stab::Object::Object()
{
    ++instances_;
}


Stab::Object::Object(const Object&)
{
    ++instances_;
}


Stab::Object::~Object()
{
    --instances_;
}


Stab::Object& Stab::Object::operator=(const Object&)
{
    return *this;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
