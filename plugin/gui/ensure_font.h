#ifndef ENSURE_FONT_H__31689BC4_FD12_4730_AFA4_179837AA09EB
#define ENSURE_FONT_H__31689BC4_FD12_4730_AFA4_179837AA09EB
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
#if GTKMM_2

#include <pangomm/font.h>
#include <gtkmm/widget.h>

bool
ensure_monospace(Pango::FontDescription&, Gtk::Widget&);

#else
 #define ensure_monospace(f, w)
#endif

#endif // ENSURE_FONT_H__31689BC4_FD12_4730_AFA4_179837AA09EB
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
