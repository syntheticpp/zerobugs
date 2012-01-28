#ifndef TEMPLATE_PARAM_H__99AD8A38_25C3_4C7C_8A2D_FEA92C5F9735
#define TEMPLATE_PARAM_H__99AD8A38_25C3_4C7C_8A2D_FEA92C5F9735
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

#include "zdk/ref_counted_impl.h"
#include "zdk/template_param.h"
#include "zdk/weak_ptr.h"


class ZDK_LOCAL TemplateTypeParamImpl
    : public RefCountedImpl<TemplateTypeParam>
{
public:
    TemplateTypeParamImpl(SharedString*, DataType*);

BEGIN_INTERFACE_MAP(TemplateTypeParamImpl)
    INTERFACE_ENTRY(TemplateTypeParam)
END_INTERFACE_MAP()

    DataType* type() const { return type_.ref_ptr().get(); }

    SharedString* name() const { return name_.get(); }

private:
    RefPtr<SharedString> name_;
    WeakPtr<DataType> type_;
};


/* todo

class TemplateValueParamImpl : public TemplateValueParam
{
    TemplateValueParamImpl();
public:

BEGIN_INTERFACE_MAP(TemplateValueParamImpl)
    INTERFACE_ENTRY(TemplateValueParam)
    INTERFACE_ENTRY(Variant)
END_INTERFACE_MAP()
};

*/
#endif // TEMPLATE_PARAM_H__99AD8A38_25C3_4C7C_8A2D_FEA92C5F9735
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
