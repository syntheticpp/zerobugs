#ifndef FONT_H__4BFBFFBA_A0C0_403C_84EF_BC1A9D9B02A1
#define FONT_H__4BFBFFBA_A0C0_403C_84EF_BC1A9D9B02A1
//
// $Id: font.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#ifdef GTKMM_2

 #include <pangomm/font.h>
 typedef Pango::FontDescription Gdk_Font;

#else

 #include <gdk--/font.h>

#endif
#endif // FONT_H__4BFBFFBA_A0C0_403C_84EF_BC1A9D9B02A1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
