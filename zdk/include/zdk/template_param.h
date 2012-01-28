#ifndef TEMPLATE_PARAM_H__F99E4F4F_ADFC_4966_B0FD_850B25F898C3
#define TEMPLATE_PARAM_H__F99E4F4F_ADFC_4966_B0FD_850B25F898C3
//
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
#include "zdk/data_type.h"
#include "zdk/variant.h"


DECLARE_ZDK_INTERFACE_(TemplateValueParam, Variant)
{
    DECLARE_UUID("b7934b79-d1ec-4224-b256-ff46870133d1")

    virtual DataType* type() const = 0;

    virtual SharedString* name() const = 0;
};


DECLARE_ZDK_INTERFACE_(TemplateTypeParam, ZObject)
{
    DECLARE_UUID("a50ee15a-5ca7-4a4d-b760-a10bab136b3a")

    virtual DataType* type() const = 0;

    virtual SharedString* name() const = 0;
};

#endif // TEMPLATE_PARAM_H__F99E4F4F_ADFC_4966_B0FD_850B25F898C3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
