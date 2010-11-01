#ifndef COLOR_H__B62A01D1_7BE1_4210_85F1_3E17C01885CD
#define COLOR_H__B62A01D1_7BE1_4210_85F1_3E17C01885CD
//
// $Id: color.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#if GTKMM_2
 #include <gdkmm/color.h>

 class Gdk_Color : public Gdk::Color
 {
    std::string name_;

 public:
    explicit Gdk_Color(const std::string& color)
        : Gdk::Color(color), name_(color)
    { }

    Gdk_Color() { }

    const std::string& name() const { return name_; }
 };
#endif

#endif // COLOR_H__B62A01D1_7BE1_4210_85F1_3E17C01885CD
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
