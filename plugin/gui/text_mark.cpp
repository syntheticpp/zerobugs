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
typedef GtkTextMark GtkSourceMarker;

GtkSourceMarker*
get_marker(const SrcBufPtr& buf, const string& name)
{
    return gtk_text_buffer_get_mark(GTK_TEXT_BUFFER(buf->gobj()), name.c_str());
}

GtkSourceMarker*
create_marker(const SrcBufPtr& buf,
              const string& name,
              const char* type,
              Gtk::SourceBuffer::iterator iter)
{
    return gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(buf->gobj()),
                                       name.c_str(), iter.gobj(), true);
}

void
delete_marker(const SrcBufPtr& buf, GtkSourceMarker* mark)
{
    gtk_text_buffer_delete_mark(GTK_TEXT_BUFFER(buf->gobj()), mark);
}


void
move_marker( const SrcBufPtr& buf,
             GtkSourceMarker* mark,
             Gtk::SourceBuffer::iterator iter)
{
    gtk_text_buffer_move_mark(GTK_TEXT_BUFFER(buf->gobj()), mark, iter.gobj());
}


#else

GtkSourceMarker*
get_marker(const SrcBufPtr& buf, const string& name)
{
    return gtk_source_buffer_get_marker(buf->gobj(), name.c_str());
}

GtkSourceMarker*
create_marker(const SrcBufPtr& buf,
              const string& name,
              const char* type,
              Gtk::SourceBuffer::iterator iter)
{
    return gtk_source_buffer_create_marker(buf->gobj(),
                                           name.c_str(),
                                           type,
                                           iter.gobj());
}

void
delete_marker(const SrcBufPtr& buf, GtkSourceMarker* mark)
{
    gtk_source_buffer_delete_marker(buf->gobj(), mark);
}

void move_marker(const SrcBufPtr& buf,
                 GtkSourceMarker* mark,
                 Gtk::SourceBuffer::iterator iter)
{
    gtk_source_buffer_move_marker(buf->gobj(), mark, iter.gobj());
}


#endif

