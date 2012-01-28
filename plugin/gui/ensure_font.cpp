// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#if GTKMM_2
#include "ensure_font.h"

bool
ensure_monospace(Pango::FontDescription& font, Gtk::Widget& w)
{
    bool success = false;
    if (Glib::RefPtr<Pango::Context> ctxt = w.get_pango_context())
    {
        Glib::ArrayHandle<Glib::RefPtr<Pango::FontFamily> > ff =
           ctxt->list_families();

        Glib::ArrayHandle<
            Glib::RefPtr<Pango::FontFamily> >::const_iterator
            i = ff.begin(), end = ff.end();

        for (; i != end; ++i)
        {
            if ((*i)->is_monospace())
            {
                font = Pango::FontDescription((*i)->get_name());
                success = true;

                w.ensure_style();
                if (Glib::RefPtr<Gtk::Style> style = w.get_style())
                {
                    font.merge(style->get_font(), false);
                }
                if (Glib::RefPtr<Pango::Font> f = ctxt->load_font(font))
                {
                    font = f->describe();
                }
                break;
            }
        }
    }
    return success;
}
#endif
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
