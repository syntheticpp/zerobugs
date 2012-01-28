#ifndef PANED_H__9B0CBE0C_8646_4AA7_BBCE_CBED7F9CFE87
#define PANED_H__9B0CBE0C_8646_4AA7_BBCE_CBED7F9CFE87
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

#include "generic/adapt_ptr.h"

#if !defined ( GTKMM_2 )
 #include <gtk--/paned.h>

 struct Paned_Adapt : public Gtk::Paned
 {
    int get_position() const
    {
        int pos = get_gutter_size() / 2;
        if (Widget* w = get_child1())
        {
            if (Gtk::HPaned::isA(const_cast<Paned_Adapt*>(this)))
            {
                pos += w->width();
            }
            else
            {
                assert (Gtk::VPaned::isA(const_cast<Paned_Adapt*>(this)));
                pos += w->height();
            }
        }
        return pos;
    }
 };

#else
 #include <gtkmm/paned.h>

 struct Paned_Adapt : public Gtk::Paned
 {
    void set_gutter_size(int) { }
 };

#endif

typedef APtr<Gtk::Paned, Paned_Adapt> PanedPtr;

enum
{
    resizeNo = false,
    resizeOk = true,
    shrinkNo = false,
    shrinkOk = true
};


#endif // PANED_H__9B0CBE0C_8646_4AA7_BBCE_CBED7F9CFE87
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
