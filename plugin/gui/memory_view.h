#ifndef MEMORY_VIEW_H__2482D190_5622_4853_ABD5_568B99136E9A
#define MEMORY_VIEW_H__2482D190_5622_4853_ABD5_568B99136E9A
//
// $Id: memory_view.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <vector>
#include "gtkmm/font.h"
#include "gtkmm/events.h"
#include "gtkmm/icon_mapper.h"
#include "gtkmm/text.h"
#include "gtkmm/window.h"
#include "zdk/ref_counted_impl.h"
#include "zdk/zero.h"
#include "set_cursor.h"


namespace Gtk
{
    class Button;
    class Entry;
}

class EvalEvents;
class TextEntry;

/**
 * Displays the contents of the debugged programs' memory.
 * Several memory views can be open at the same time.
 */
class MemoryView : public RefCountedImpl<>
                 , public OnMapEventImpl<Gtk::Window>
{
public:
    MemoryView(Properties*, RefPtr<Thread>, addr_t);

    virtual ~MemoryView() throw();

    void update();  // called on main thread
    void display(); // called on ui thread

    EvalEvents& events();

    SigC::Signal1<void, RefPtr<MemoryView> > update_request;
    SigC::Signal4<bool, std::string, addr_t, ExprEvents*, int> evaluate;

#ifdef GTKMM_2
    SigC::Signal0<void>& signal_destroy()
    {
        return destroy_;
    }

private:
    SigC::Signal0<void> destroy_;

protected:
    void on_size_allocate(Gtk::Allocation&);

#else // gtk-- 1.2

protected:
    void destroy_() { }

    void size_allocate_impl(GtkAllocation*);

#endif
    event_result_t delete_event_impl(GdkEventAny*);

    void show_all_impl();

    void activate();

    void get_geometry(int& nrows, int& ncols);

    void toggle_freeze();

    void update_impl(bool request = true);

    void display_impl(int nrows, int ncols);

    void on_done(const Variant&);

    bool on_error(std::string);

private:
    RefPtr<Thread>      thread_;
    addr_t              addr_;
    TextEntry*          entry_;
    Gtk::Text*          text_;
    Gtk::Button*        btn_;
    Gdk_Font            font_;
    std::vector<long>   data_;
    std::vector<long>   oldData_;
    std::string         error_;
    bool                frozen_;
    bool                pending_;

#ifdef GTKMM_2
    Gdk::Rectangle      rect_;
#endif
    RefPtr<EvalEvents>  events_;
};

#endif // MEMORY_VIEW_H__2482D190_5622_4853_ABD5_568B99136E9A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
