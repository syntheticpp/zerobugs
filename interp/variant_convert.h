#ifndef VARIANT_CONVERT_H__CFED1CE9_6589_4F24_B562_732BB95BE596
#define VARIANT_CONVERT_H__CFED1CE9_6589_4F24_B562_732BB95BE596
//
// $Id: variant_convert.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/variant.h"

class DataType;

RefPtr<Variant> variant_convert(RefPtr<Variant>&, DataType&);

#endif // VARIANT_CONVERT_H__CFED1CE9_6589_4F24_B562_732BB95BE596
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
