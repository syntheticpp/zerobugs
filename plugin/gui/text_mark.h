#ifndef TEXT_MARK_H__AA24010B_DAA7_4D02_A4D7_1002443773D8
#define TEXT_MARK_H__AA24010B_DAA7_4D02_A4D7_1002443773D8
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: text_mark.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#ifdef GTKMM_2

#include <string>
#include "gtksourceview/gtksourcebuffer.h"
#include "gtksourceview/gtksourceview.h"
#include "gtkmm/text.h"


//
// Marker stuff may be missing fron Gtk::SourceBuffer,
// depending on the version of gtksourceviewmm
//
typedef Glib::RefPtr<Gtk::SourceBuffer> SrcBufPtr;

GtkSourceMarker*
get_marker(const SrcBufPtr& buf, const std::string& name);


GtkSourceMarker*
create_marker(const SrcBufPtr& buf,
              const std::string& name,
              const char* type,
              Gtk::SourceBuffer::iterator iter);

void
delete_marker(const SrcBufPtr& buf, GtkSourceMarker* mark);

void
move_marker (const SrcBufPtr& buf,
             GtkSourceMarker* mark,
             Gtk::SourceBuffer::iterator iter);

#endif // GTKMM_2
#endif // TEXT_MARK_H__AA24010B_DAA7_4D02_A4D7_1002443773D8
