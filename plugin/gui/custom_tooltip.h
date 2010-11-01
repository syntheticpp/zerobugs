#ifndef CUSTOM_TOOLTIP_H__048A1A68_DD9D_4881_A1A8_072518D91A51
#define CUSTOM_TOOLTIP_H__048A1A68_DD9D_4881_A1A8_072518D91A51
//
// $Id: custom_tooltip.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <memory>
#include <string>
#include "gtkmm/base.h"
#include "gtkmm/clist.h"
#include "gtkmm/ctree.h"
#include "gtkmm/events.h"
#include "line_wrap.h"

namespace Gtk
{
    class CList;
    class CTree;
    class Widget;
    class Window;
}


typedef bool (*GetTextAtPointer)(
    Gtk::Widget&,
    double  x,  // cursor coordinates
    double  y,
    Gtk::RowHandle&,
    int&    column,
    std::string& // output string
);


/**
 * Decorates a widget with a custom tooltip.
 */
class ZDK_LOCAL CustomToolTip : public Gtk::Base
{
public:
    CustomToolTip(Gtk::Widget*, GetTextAtPointer);

    ~CustomToolTip();

    int on_paint_custom_tip(GdkEventExpose*);

    int on_pointer_motion(GdkEventMotion*);

    int on_pointer_leave(GdkEventCrossing*);

    void reset_tooltip();

protected:
    void position_tooltip(int, int);

private:
    // non-copyable, non-assignable
    CustomToolTip(const CustomToolTip&);
    CustomToolTip& operator=(const CustomToolTip&);

    // the decorated widget
    Gtk::Widget* widget_;

    GetTextAtPointer getTextAtPointer_;
    std::auto_ptr<Gtk::Window> tooltip_;

    Gtk::RowHandle hrow_;
    int hcol_;
    bool drawing_;
};


template<typename T>
struct ZDK_LOCAL ToolTipTraits
{
    static bool get_text_at_pointer(
        Gtk::Widget&    wid,
        double          x,
        double          y,
        Gtk::RowHandle& hrow,
        int&            hcol,
        std::string&    text)
    {
        bool result = false;
        T& tw = dynamic_cast<T&>(wid);
        Gtk::RowHandle nrow;
        int ncol = 0;

        if (tw.get_selection_info((int)x, (int)y, &nrow, &ncol))
        {
            // same row/cell as last time?
            if (hcol != ncol || !ROW_HANDLE_EQUAL(hrow, nrow))
            {
                // no, memorize row/col and get text at cell
                hcol = ncol, hrow = nrow;
                text = line_wrap(tw.rows()[nrow][ncol].get_text(), 64);

                result = true;
            }
        }
        return result;
    }
};


template<typename T, typename Traits = ToolTipTraits<T> >
class  ZDK_LOCAL ToolTipped : public T
{
public:
    ToolTipped() : tooltip_(this, Traits::get_text_at_pointer)
    {}

    template<typename U>
    explicit ToolTipped(U arg)
        : T(arg)
        , tooltip_(this, Traits::get_text_at_pointer)
    {}

    void reset_tooltip() { tooltip_.reset_tooltip(); }

    event_result_t on_motion_notify_event(GdkEventMotion* event)
    {
        event_result_t result = T::on_motion_notify_event(event);
        if (result && event)
        {
            result = tooltip_.on_pointer_motion(event);
        }
        return result;
    }

    event_result_t on_leave_notify_event(GdkEventCrossing* event)
    {
        event_result_t tmp = T::on_leave_notify_event(event);
        return tooltip_.on_pointer_leave(event) && tmp;
    }

#ifdef GTKMM_2
    event_result_t on_button_press_event(GdkEventButton* event)
    {
        T::on_button_press_event(event);
        return false;
    }
#endif

private:
    CustomToolTip tooltip_;
};

// Copyright (c) 2004, 2005 Cristian L. Vlasceanu

#endif // CUSTOM_TOOLTIP_H__048A1A68_DD9D_4881_A1A8_072518D91A51
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
