#ifndef EVENTS_H__54AAADFE_0F64_431A_B5D6_87148AB58A65
#define EVENTS_H__54AAADFE_0F64_431A_B5D6_87148AB58A65
//
// $Id: events.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#if GTKMM_2
 typedef bool event_result_t;
#else
 #include <gtk--/widget.h>

 typedef int event_result_t;

 #define on_button_press_event  button_press_event_impl
 #define on_delete_event        delete_event_impl
 #define on_selection_done      selection_done_impl
 #define on_motion_notify_event motion_notify_event_impl
 #define on_leave_notify_event  leave_notify_event_impl

 #define on_size_request        size_request_impl
 #define on_size_allocation     size_allocate_impl

 namespace Gtk
 {
    typedef GtkRequisition Requisition;
    typedef GtkAllocation Allocation;
 }
#endif
#endif // EVENTS_H__54AAADFE_0F64_431A_B5D6_87148AB58A65
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
