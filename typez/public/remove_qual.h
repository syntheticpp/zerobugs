#ifndef REMOVE_QUAL_H__BF46AEA2_0FC2_46ED_B048_D2D2A54C8C2B
#define REMOVE_QUAL_H__BF46AEA2_0FC2_46ED_B048_D2D2A54C8C2B
//
// $Id: remove_qual.h 714 2010-10-17 10:03:52Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/ref_ptr.h"
#include "zdk/types.h"

/// remove cv-qualifiers
RefPtr<DataType> remove_qualifiers(RefPtr<DataType>);

DataType* remove_qualifiers(DataType*);

#endif // REMOVE_QUAL_H__BF46AEA2_0FC2_46ED_B048_D2D2A54C8C2B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
