#ifndef TEXT_MARK_H__AA24010B_DAA7_4D02_A4D7_1002443773D8
#define TEXT_MARK_H__AA24010B_DAA7_4D02_A4D7_1002443773D8
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#ifdef GTKMM_2
#include "gtkmm/text.h"

typedef Glib::RefPtr<Gtk::SourceBuffer> SrcBufPtr;


void
create_marker (
    SrcBufPtr buffer,
    Gtk::SourceBuffer::iterator  where,
    size_t line,
    const char* category );


void
remove_marker (
    SrcBufPtr buffer,
    Gtk::SourceBuffer::iterator  where,
    size_t line,
    const char* category );

#endif // GTKMM_2
#endif // TEXT_MARK_H__AA24010B_DAA7_4D02_A4D7_1002443773D8
