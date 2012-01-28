#ifndef ADD_ACCEL_H__27F4E995_91A2_4F18_AC71_F45C64097C8F
#define ADD_ACCEL_H__27F4E995_91A2_4F18_AC71_F45C64097C8F
//
// $Id$
//
#ifdef GTKMM_2
 #define Gtk_ADD_ACCEL(w,s,ag,k,mod,f) \
    (w).add_accelerator(Glib::ustring(s),(ag),(k),(mod),(f))
#else
 #define Gtk_ADD_ACCEL(w,s,ag,k,mod,f) \
    (w).add_accelerator((s),*(ag),(k),(mod),(f))
#endif
#endif // ADD_ACCEL_H__27F4E995_91A2_4F18_AC71_F45C64097C8F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
