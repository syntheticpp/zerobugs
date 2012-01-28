// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "config.h"
#include "dharma/environ.h"
#include "notebook.h"
#include <iostream>
#include <vector>
#include <gtkmm/style.h>
#if HAVE_GDK_CAIRO_SUPPORT
 #include <cairomm/context.h>
 #include <cairomm/pattern.h>
#endif

using namespace Gdk;
using namespace Gtk;
using namespace std;


#if HAVE_GDK_CAIRO_SUPPORT
static const size_t SLANT = 30;

namespace
{
    class ZDK_LOCAL CustomNotebookStyle : public Style
    {
        vector<GdkPoint> mask_, last_;
        PositionType side_;

        Glib::RefPtr<Gtk::Style> clone_vfunc ();

        void draw_extension_vfunc(
            const Glib::RefPtr<Gdk::Window>&,
            Gtk::StateType,
            Gtk::ShadowType,
            const Gdk::Rectangle& area,
            Gtk::Widget* widget,
            const Glib::ustring& detail,
            int x, int y, int width, int height,
            PositionType gapSide);

        void draw_box_gap_vfunc(
            const Glib::RefPtr<Gdk::Window>&,
            Gtk::StateType,
            Gtk::ShadowType,
            const Gdk::Rectangle& area,
            Gtk::Widget* widget,
            const Glib::ustring& detail,
            int x, int y, int width, int height,
            PositionType gapSidei,
            int, int);

        ShadowType selected_shadow() const
        {
            ShadowType shadow = SHADOW_IN;
            if (side_ == POS_BOTTOM)
            {
                shadow = SHADOW_ETCHED_OUT;
            }
            return shadow;
        }

    public:
        CustomNotebookStyle() : side_(POS_BOTTOM) { }

        using Style::copy_vfunc;

        void draw_tab(
                  const Glib::RefPtr<Gdk::Window>& w,
                  const Gdk::Rectangle& r,
                  Widget* widget)
         {
            bool drawFocus = false;
            // the window is not ours when the tab is being dragged
            if (w != widget->get_window())
            {
                mask_.clear();
                mask_.resize(4);
                mask_[0].x = r.get_x();
                mask_[0].y = r.get_y();
                mask_[1].x = r.get_x() + r.get_width();
                mask_[1].y = r.get_y();
                mask_[2].x = r.get_x() + r.get_width();
                mask_[2].y = r.get_y() + r.get_height();
                mask_[3].x = r.get_x();
                mask_[3].y = r.get_y() + r.get_height();
                drawFocus = true;
            }
            if (w && !mask_.empty())
            {
                ShadowType shadow = selected_shadow();
                draw_polygon_vfunc(w,
                                   STATE_NORMAL,
                                   shadow,
                                   r,
                                   widget,
                                   "tab",
                                   &mask_[0],
                                   mask_.size(),
                                   true);
                mask_.clear();

                if (drawFocus)
                {
                    draw_focus_vfunc(w,
                                    STATE_NORMAL,
                                    r,
                                    widget,
                                    "tab",
                                    3, 2,
                                    r.get_width() - 6,
                                    r.get_height() - 2);
                }
            }
            else if (!last_.empty())
            {
                draw_polygon_vfunc(w, STATE_NORMAL,
                           selected_shadow(),
                           r, widget, "tab",
                           &last_[0], last_.size(),
                           true);
            }
        }
    };
} // namespace


Glib::RefPtr<Style> CustomNotebookStyle::clone_vfunc()
{
    Glib::RefPtr<CustomNotebookStyle> style(new CustomNotebookStyle);
    style->copy_vfunc(Glib::RefPtr<Style>(this));

    return style;
}


/**
 * Fillout vector of points for the shape to drawing the selected tab in.
 */
static void
get_mask(Widget*         w,
         int             x,
         int             y,
         int             width,
         int             height,
         PositionType    pos,
         vector<Point>&  shape)
{
    assert(shape.empty());

    if (pos == POS_BOTTOM)
    {
    #if 0
        ++x;
        ++y;

        const int xRight = x + width + height;
        shape.push_back(Point(x, y + height));
        shape.push_back(Point(x, y + 2));
        shape.push_back(Point(x + 2, y));
        shape.push_back(Point(x + width - 6, y));
        shape.push_back(Point(x + width - 4, y + 1));
        shape.push_back(Point(x + width - 3, y + 1));
        shape.push_back(Point(x + width + 2, y + 4));
        shape.push_back(Point(xRight - 4, y + height - 2));
        shape.push_back(Point(xRight - 1, y + height - 1));
        shape.push_back(Point(xRight + 1, y + height));
        shape.push_back(Point(x + width + SLANT, y + height));
    #endif
    }
    else if (pos == POS_TOP)
    {
        --y;
        --height;

        shape.push_back(Point(x, y));
        shape.push_back(Point(x + width + height, y));
        shape.push_back(Point(x + width, y + height));
        shape.push_back(Point(x, y + height));
    }
}


static void
get_mask(Widget*            w,
         int                x,
         int                y,
         int                width,
         int                height,
         PositionType       pos,
         vector<GdkPoint>&  poly)
{
    vector<Point> shape;
    get_mask(w, x, y, width, height, pos, shape);
    for (vector<Point>::const_iterator i = shape.begin(); i != shape.end(); ++i)
    {
        poly.push_back(*i->gobj());
    }
}



void
CustomNotebookStyle::draw_extension_vfunc(
    const Glib::RefPtr<Gdk::Window>& window,
    Gtk::StateType state,
    ShadowType shadow,
    const Rectangle& area,
    Widget* widget,
    const Glib::ustring& detail,
    int x, int y, int width, int height,
    PositionType gapSide)
{
    vector<GdkPoint> poly;

    if (state == STATE_NORMAL)
    {
        // save the shape of the active tab for later
        get_mask(widget, x, y, width, height, gapSide, mask_);
        side_ = gapSide;
        last_ = mask_;
    }

    if (mask_.empty())
    {
        Style::draw_extension_vfunc(window,
                                    state,
                                    shadow,
                                    area,
                                    widget,
                                    detail,
                                    x, y, width, height,
                                    gapSide);

        Cairo::RefPtr<Cairo::Context> context = window->create_cairo_context();
        Gdk::Color begin = get_light(state);
        Gdk::Color end = get_base(state);

        Cairo::RefPtr<Cairo::LinearGradient> gradient;
        if (gapSide == POS_TOP)
        {
            gradient = Cairo::LinearGradient::create(x, y, x, y + height);
        }
        else
        {
            gradient = Cairo::LinearGradient::create(x, y + height, x, y);
        }
        // Set grandient colors
        gradient->add_color_stop_rgb(0,
                begin.get_red_p(),
                begin.get_green_p(),
                begin.get_blue_p());
        gradient->add_color_stop_rgb(1,
                end.get_red_p(),
                end.get_green_p(),
                end.get_blue_p());
        context->set_source(gradient);
        if (gapSide == POS_TOP)
        {
            height -= 2;
        }
        else
        {
            y += 3;
            height -= 3;
        }
        context->rectangle(x + 2, y, width - 4, height);
        context->fill();
    }
}


void CustomNotebookStyle::draw_box_gap_vfunc(
        const Glib::RefPtr<Gdk::Window>& window,
        Gtk::StateType state,
        Gtk::ShadowType shadow,
        const Gdk::Rectangle& area,
        Gtk::Widget* widget,
        const Glib::ustring& detail,
        int x, int y, int width, int height,
        PositionType gapSide,
        int gapX, int gapWidth)
{
    if (gapSide == POS_TOP)
    {
        if (!mask_.empty())
        {
            gapWidth += SLANT;
        }
        --width, --height;
        Glib::RefPtr<GC> gc = GC::create(window);
        gc->set_foreground(get_dark(state));
        vector<Point> points;

        points.push_back(Point(x + gapX + gapWidth, y));
        points.push_back(Point(x + width, y));
        points.push_back(Point(x + gapX + gapWidth, y - 1));
        points.push_back(Point(x + width, y - 1));
        points.push_back(Point(x + width, y + height));
        points.push_back(Point(x, y + height));
        points.push_back(Point(x, y));
        points.push_back(Point(x + gapX, y));

        window->draw_lines(gc, points);
    }
    else
    {
        Style::draw_box_gap_vfunc(window,
                               state,
                               shadow,
                               area,
                               widget,
                               detail,
                               x, y, width, height,
                               gapSide,
                               gapX, gapWidth);
    }
}
#endif // HAVE_GDK_CAIRO_SUPPORT


CustomNotebook::CustomNotebook() : Notebook()
{
//    Gdk::Window::set_debug_updates(true);
}



void CustomNotebook::set_custom_style()
{
#if HAVE_GDK_CAIRO_SUPPORT
    Glib::RefPtr<Style> oldStyle = get_style();
    Glib::RefPtr<CustomNotebookStyle> style(new CustomNotebookStyle);
    style->copy_vfunc(oldStyle);
    style->set_light(STATE_ACTIVE, oldStyle->get_light(STATE_ACTIVE));
    style->set_mid(STATE_ACTIVE, oldStyle->get_mid(STATE_ACTIVE));
    style->set_dark(STATE_ACTIVE, oldStyle->get_dark(STATE_ACTIVE));
    style->set_light(STATE_NORMAL, oldStyle->get_light(STATE_NORMAL));
    style->set_mid(STATE_NORMAL, oldStyle->get_mid(STATE_NORMAL));
    style->set_dark(STATE_NORMAL, oldStyle->get_dark(STATE_NORMAL));

    set_style(style);
#endif // HAVE_GDK_CAIRO_SUPPORT
}


static bool use_custom_theme()
{
    static bool custom =  env::get_bool("ZERO_CUSTOM_THEME", true);
    return custom;
}


bool CustomNotebook::on_map_event(GdkEventAny* event)
{
    bool ret = Notebook::on_map_event(event);
    if (use_custom_theme())
    {
        set_custom_style();
    }
    return ret;
}


bool CustomNotebook::on_expose_event(GdkEventExpose* event)
{
    bool result = Notebook::on_expose_event(event);
#if HAVE_GDK_CAIRO_SUPPORT
    Rectangle rect(&event->area);

    if (Glib::RefPtr<CustomNotebookStyle> style =
        Glib::RefPtr<CustomNotebookStyle>::cast_dynamic(get_style()))
    {
        style->draw_tab(
            Glib::wrap(GDK_WINDOW_OBJECT(event->window), true),
            rect, this);
    }

    int curPageNum = get_current_page();
    if (curPageNum != -1)
    {
        if (Widget* child = pages()[curPageNum].get_tab_label())
        {
            propagate_expose(*child, event);
        }
    }
#endif // HAVE_GDK_CAIRO_SUPPORT
    return result;
}


void CustomNotebook::on_size_request(Requisition* requisition)
{
    Notebook::on_size_request(requisition);
}


void CustomNotebook::on_size_allocate(Allocation& allocation)
{
    Notebook::on_size_allocate(allocation);
}


void CustomNotebook::on_style_changed(const Glib::RefPtr<Style>& prev)
{
    Notebook::on_style_changed(prev);
}


int CustomNotebook::append_page(Widget& w, const Glib::ustring& labelText)
{
#if HAVE_GDK_CAIRO_SUPPORT
    if (use_custom_theme())
    {
        return Notebook::append_page(w, "    " + labelText, false);
    }
#endif
    return Notebook::append_page(w, labelText, false);
}


void
CustomNotebook::set_tab_label_text(Widget& w, const Glib::ustring& labelText)
{
#if HAVE_GDK_CAIRO_SUPPORT
    if (use_custom_theme())
    {
        Notebook::set_tab_label_text(w, "    " + labelText);
    }
    else
#endif
        Notebook::set_tab_label_text(w, labelText);
}
