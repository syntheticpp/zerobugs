//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: text_mark.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "config.h"
#include "text_mark.h"

using namespace std;

#if (GTKSVMM_API_VERSION >= 2)
void
create_marker (
    SrcBufPtr buf,
    Gtk::SourceBuffer::iterator  iter,
    size_t line,
    const char* category )
{
    if (buf->get_source_marks_at_iter(iter, category).empty())
    {
        ostringstream name;
        name << "L." << line;

        buf->create_source_mark(name.str(), category, iter);
    }
}


void
remove_marker (
    SrcBufPtr buf,
    Gtk::SourceBuffer::iterator  where,
    size_t line,
    const char* category )
{
    auto next = where;
    ++next;

    buf->remove_source_marks(where, next, category);
}


void
remove_markers (
    SrcBufPtr buf,
    const char* category )
{
    buf->remove_source_marks(buf->begin(), buf->end(), category);
}


#else


void
create_marker (
    SrcBufPtr buf,
    Gtk::SourceBuffer::iterator  iter,
    size_t /* line */,
    const char* type )
{
    ostringstream name;
    name << "L." << line;

    gtk_source_buffer_create_marker(buf->gobj(),
                                    name.str().c_str(),
                                    type,
                                    iter.gobj());
}

void
remove_marker (
    SrcBufPtr buf,
    Gtk::SourceBuffer::iterator  /* where */,
    size_t line,
    const char* category )
{
    ostringstream name;
    name << "L." << line;

    if (GtkSourceMarker* m = gtk_source_buffer_get_marker(buf->gobj(), name.str(), c_str()))
    {
        gtk_source_buffer_delete_marker(buf->gobj(), mark);
    }
}


void
remove_markers (
    SrcBufPtr buffer,
    const char* category )
{
    assert(false); // TODO
}

#endif

