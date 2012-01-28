#ifndef TOOLBAR_H__02B47958_12DD_4613_8066_56DB05BB2C82
#define TOOLBAR_H__02B47958_12DD_4613_8066_56DB05BB2C82
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

#include "gtkmm/image.h"
#include "gtkmm/pixmap.h"
#include "gtkmm/stock.h"
#include "gtkmm/toolbar.h"
#include "zdk/check_ptr.h"
#include "states.h"


class ZDK_LOCAL ToolBar : public Gtk::Toolbar
{
public:
    ToolBar()
    {
#ifdef GTKMM_2
        set_tooltips(true);
        set_toolbar_style(Gtk::TOOLBAR_BOTH_HORIZ);
#endif
    }
    template<typename F>
    Gtk::Widget* add_button
    (
        const char* const* pixData,
        const char* tip,
        const F& fn,
        unsigned stateMask,
        const char* text,
        const char* stock,
        bool important = true
    )
    {
    #ifdef GTKMM_2
        Gtk::Image* icon = NULL;
        if (stock && *stock)
        {
            icon = new Gtk::Image(Gtk::StockID(stock), Gtk::ICON_SIZE_SMALL_TOOLBAR);

            if (icon->get_storage_type() == Gtk::IMAGE_EMPTY)
            {
                delete icon;
                icon = NULL;
            }
            else
            {
                manage(icon);
            }
        }
    #else
        Gtk::Pixmap* icon = NULL;
    #endif
        if (!icon)
        {
            icon = manage(new Gtk::Pixmap(pixData));
        }
    #ifdef GTKMM_2
        Gtk::ToolButton* btn = manage(new Gtk::ToolButton(*icon, text));
        if (important)
        {
            btn->set_is_important();
        }
        append(*btn);

        if (Gtk::Tooltips* tips = get_tooltips_object())
        {
             btn->set_tooltip(*tips, tip);
        }
        else // The Tooltips API was deprecated in Gtk 2.12
        {
             btn->set_tooltip_text(tip);
        }

        btn->signal_clicked().connect(fn);

    #else
        icon->set_build_insensitive(true);
        tools().push_back(
            Gtk::Toolbar_Helpers::ButtonElem(text, *icon, fn, tip, ""));

        Gtk::Widget* btn = tools().back()->get_widget();
    #endif
        CHKPTR(btn)->show();
        btn->set_data(STATE_MASK, reinterpret_cast<void*>(stateMask));
        return btn;
    }

    void add_separator();

protected:
    template<typename T>
    Gtk::Widget* add_button(
        const char* pixmapData[],
        const char* tip,
        void (T::*fn)(),
        unsigned int stateMask = 0,
        const char* name = 0,
        const char* stock = 0,
        bool important = true)
    {
        return add_button(pixmapData, tip,
                          Gtk_SLOT(static_cast<T*>(this), fn),
                          stateMask,
                          name, stock,
                          important);
    }
};
#endif // TOOLBAR_H__02B47958_12DD_4613_8066_56DB05BB2C82
