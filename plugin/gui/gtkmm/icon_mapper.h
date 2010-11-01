#ifndef ICON_MAPPER_H__B7038034_F411_4821_86CA_20F2719B2A22
#define ICON_MAPPER_H__B7038034_F411_4821_86CA_20F2719B2A22
//
// $Id: icon_mapper.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/export.h"
#include "pixmap.h"

#ifndef ICON_DEFINED
 #include "../icons/zeroicon.xpm"
 #define ICON_DEFINED 1
#endif

#ifdef GTKMM_2
#include <gdkmm/cursor.h>
#include <gdkmm/window.h>

 template<typename T>
 class ZDK_LOCAL OnMapEventImpl : public T
 {
    PixmapPtr icon_;
    Gdk::CursorType cursorType_;

public:
    void set_cursor_type(Gdk::CursorType cursorType)
    {
        cursorType_ = cursorType;
    }

protected:
    typedef OnMapEventImpl Supper;

    OnMapEventImpl() : cursorType_(Gdk::TOP_LEFT_ARROW)
    { }

    template<typename U>
    explicit OnMapEventImpl(U arg)
        : T(arg), cursorType_(Gdk::TOP_LEFT_ARROW)
    { }

    bool on_map_event(GdkEventAny* event)
    {
        const bool result = T::on_map_event(event);

        if (!icon_)
        {
            Glib::RefPtr<Gdk::Bitmap> mask;
            Gdk::Color color("white");

            if (Glib::RefPtr<Gdk::Window> wnd = this->get_window())
            {
                Glib::RefPtr<const Gdk::Drawable> drawable = wnd;
                icon_ = Gdk::Pixmap::create_from_xpm(drawable, mask, color, zeroicon_xpm);
                wnd->set_icon(wnd, icon_, mask);
                wnd->set_cursor(Gdk::Cursor(cursorType_));
            }
        }
        return result;
    }

    PixmapPtr get_icon() const { return icon_; }

 };
#else // gtk-- 1.2
 #include <gdk--/cursor.h>

 template<typename T>
 class ZDK_LOCAL OnMapEventImpl : public T
 {
    Gdk_Pixmap icon_;
    GdkCursorType cursorType_;

public:
    void set_cursor_type(GdkCursorType cursorType)
    {
        cursorType_ = cursorType;
    }

protected:
    typedef OnMapEventImpl Supper;

    OnMapEventImpl() : cursorType_(GDK_TOP_LEFT_ARROW) { }

    template<typename U>
    OnMapEventImpl(U arg) : T(arg), cursorType_(GDK_TOP_LEFT_ARROW)
    { }

    int map_event_impl(GdkEventAny* event)
    {
        const int result = T::map_event_impl(event);

        Gdk_Bitmap mask;
        Gdk_Color  color("white");

        Gdk_Window wnd = this->get_window();

        icon_.create_from_xpm_d(wnd, mask, color, zeroicon_xpm);
        wnd.set_icon(wnd, icon_, mask);
        wnd.set_cursor(cursorType_);

        return result;
    }

    Gdk_Pixmap& get_icon() { return icon_; }
 };
#endif // GTK-- 1.2
#endif // ICON_MAPPER_H__B7038034_F411_4821_86CA_20F2719B2A22
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
