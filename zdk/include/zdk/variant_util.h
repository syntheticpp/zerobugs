#ifndef VARIANT_UTIL_H__EEEF5E1E_9945_4C86_925A_B9E4999C5845
#define VARIANT_UTIL_H__EEEF5E1E_9945_4C86_925A_B9E4999C5845
//
// $Id: variant_util.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iosfwd>
#include "zdk/variant.h"

/**
 * print the variant's value to an output stream, specifying
 * an optional numeric base
 */
std::ostream& variant_print(std::ostream&, const Variant&, int = 0);

/**
 *@return true if the variant's value is non-zero
 */
bool variant_true(const Variant& v);

const char* variant_type(Variant::TypeTag);

inline std::ostream& operator<<(std::ostream& out, const Variant& var)
{
    variant_print(out, var);
    return out;
}

#endif // VARIANT_UTIL_H__EEEF5E1E_9945_4C86_925A_B9E4999C5845
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
