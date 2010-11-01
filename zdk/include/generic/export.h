#ifndef EXPORT_H__BE70B0BF_A053_41F2_BED9_5EB7777C4FE5
#define EXPORT_H__BE70B0BF_A053_41F2_BED9_5EB7777C4FE5
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: export.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
#if !defined (ZDK_EXPORT)

#if (__GNUC__ >= 4)

 #define ZDK_EXPORT __attribute__((visibility("default")))
 #define ZDK_LOCAL __attribute__((visibility("hidden")))

#else

 #define ZDK_EXPORT
 #define ZDK_LOCAL

#endif // __GNUC__ >= 4
#endif // !defined (ZDK_EXPORT)

#define CLASS   class ZDK_LOCAL

#endif // EXPORT_H__BE70B0BF_A053_41F2_BED9_5EB7777C4FE5
