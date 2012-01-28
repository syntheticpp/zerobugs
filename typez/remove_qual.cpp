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

#include "public/remove_qual.h"


/* Strip const and volatile qualifiers off DataType */

RefPtr<DataType> 
remove_qualifiers(RefPtr<DataType> type)
{
    while (QualifiedType* qt =
        interface_cast<QualifiedType*>(type.get()))
    {
        type = qt->remove_qualifier();
        assert(type.get());
    }
    return type;
}


DataType*
remove_qualifiers(DataType* type)
{
    while (QualifiedType* qt = interface_cast<QualifiedType*>(type))
    {
        type = qt->remove_qualifier();
        assert(type);
    }
    return type;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
