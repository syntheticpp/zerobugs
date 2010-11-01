#ifndef PIXMAP_H__9E22D3F1_B8B2_4C52_9533_01C29B287C98
#define PIXMAP_H__9E22D3F1_B8B2_4C52_9533_01C29B287C98
//
// $Id: pixmap.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#ifdef GTKMM_2
 #include <gdkmm/pixmap.h>
 #include <gtkmm/image.h>

 namespace Gtk
 {
    /**
     * Fake the old gtkmm-1.2 Pixmap class with a Gtkmm 2.x Image
     */
    class Pixmap : public Image
    {
    public:
        explicit Pixmap(const char* const* data)
            : Image(Gdk::Pixbuf::create_from_xpm_data(data))
        { }
        Pixmap(const StockID& stockID, IconSize size)
            : Image(stockID, size)
        { }
    };
 }

 typedef Glib::RefPtr<Gdk::Pixmap> PixmapPtr;
 typedef Glib::RefPtr<Gdk::Bitmap> BitmapPtr;

 template<typename T>
 void inline Gtk_add_pixmap(T& bin, Gtk::Pixmap& pix)
 {
    PixmapPtr pixmap;
    BitmapPtr bitmap;

    pix.get_pixbuf()->render_pixmap_and_mask(pixmap, bitmap, 127);
    bin.add_pixmap(pixmap, bitmap);
 }
#else
 #include <gtk--/pixmap.h>

 template<typename T>
 void inline Gtk_add_pixmap(T& bin, Gtk::Pixmap& pix)
 {
    struct PixmapOverlay : public Gtk::Pixmap
    {
        Gtk::Pixmap* clone() const
        {
            return manage(new Gtk::Pixmap(data_));
        }
    };
    bin.add(*static_cast<PixmapOverlay&>(pix).clone());
 }
#endif
#endif // PIXMAP_H__9E22D3F1_B8B2_4C52_9533_01C29B287C98
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
