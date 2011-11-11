#ifndef TEXT_H__6A4228B5_6E98_4B40_A53E_3F06B2F12682
#define TEXT_H__6A4228B5_6E98_4B40_A53E_3F06B2F12682
//
// $Id: text.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "config.h"
#include "color.h"
#include "flags.h"
#include "font.h"
#include "textfwd.h"
#include "zdk/export.h"
#include <boost/utility.hpp>
#include <iostream>

#if !defined(GTKMM_2)
// gtk-- 1.2
 #include <gtk--/text.h>

#else
 #include <gtkmm/textview.h>
 #include <gtksourceviewmm.h>
 #include <gtksourceview/gtksourceview.h>

 namespace gtksourceview
 {
 #if (GTKSVMM_API_VERSION < 2)
   typedef SourceLanguagesManager SourceLanguageManager;
 #endif
 }


 namespace Gtk
 {
    using namespace gtksourceview;

    class Text;

    struct ZDK_LOCAL TextContext
    {
        Gdk_Font font_;
        Gdk_Color fg_;
        Gdk_Color bg_;
        bool fontSet_, fgSet_, bgSet_;

        explicit TextContext(Text&)
            : fontSet_(false), fgSet_(false), bgSet_(false) { }

        void set_font(const Gdk_Font& f) { font_ = f; fontSet_ = true; }

        void set_foreground(const Gdk_Color& fg) { fg_ = fg; fgSet_ = true; }

        void set_background(const Gdk_Color& bg) { bg_ = bg; bgSet_ = true; }
    };


    /**
     * Adapt SourceView into the old gtk-- 1.2 Text
     */
    class ZDK_LOCAL Text : public SourceView
    {
    public:
        typedef TextContext Context;

        class NotUndoableScope : boost::noncopyable
        {
            Glib::RefPtr<SourceBuffer> buf_;

        public:
            explicit NotUndoableScope(Glib::RefPtr<SourceBuffer> buf)
                : buf_(buf)
            {
                if (buf_) buf_->begin_not_undoable_action();
            }
            ~NotUndoableScope()
            {
                if (buf_) buf_->end_not_undoable_action();
            }
        };

        Text();
        virtual ~Text();

        using Widget::get_window;

        size_t get_length() const;

        std::string get_chars(gint startPos = 0, gint endPos = -1) const;

        /**
         * Sets the cursor at the given point.
         * In this case a point constitutes the number of characters from the
         * extreme upper left corner of the Gtk::Text widget.
         * @param index The number of characters from the upper left corner.
         */
        void set_point(guint index);

        /**
         * Gets the current position of the cursor as the number of characters
         * from the upper left corner of the Gtk::Text widget.
         * @return The number of characters from the upper left corner.
         */
        guint get_point() const;

        /**
         * Retrieves the current cursor position.
         * @return the position of the cursor. The cursor is displayed before
         * the character with the given (base 0) index in the widget. The value
         * will be less than or equal to the number of characters in the widget.
         * Note that this position is in characters, not in bytes.
         */
        gint get_position() const;

        void insert(const Pango::FontDescription&,
                    const Gdk_Color& foregnd,
                    const Gdk_Color& backgnd,
                    const std::string& text,
                    int position);

        /**
         * @note the text string is NOT normalized, to maximize
         * performance.  It's up to the caller code to do so.
         */
        Gtk::SourceBuffer::iterator insert(
            const Pango::FontDescription&,
            const Glib::ustring& text);

        /**
         * @note the text string is NOT normalized, to maximize
         * performance.  It's up to the caller code to do so.
         */
        void insert(Context&, const Glib::ustring&);

       /**
        * Deletes from the current point position forward the given number
        * of characters.
        * @param nchars The number of characters to delete.
        * @return true if the operation was successful, otherwise returns false.
        */
        bool forward_delete(guint nchars);

        bool delete_text(gint, gint);

        void select_region(gint start, gint end);

        bool has_selection() const;

        size_t get_selection_start_pos() const;

        size_t get_selection_end_pos() const;

        void set_line_wrap(bool wrap)
        {
            set_wrap_mode(wrap ? Gtk::WRAP_WORD : Gtk::WRAP_NONE);
        }

        std::string get_word_at_iter(Gtk::TextBuffer::iterator);

        Context get_context() { return Context(*this); }

        int scroll(size_t line);

        void set_language(const char*);

        Glib::RefPtr<SourceBuffer> get_buffer();

        Glib::RefPtr<const SourceBuffer> get_buffer() const
        {
            return const_cast<Text*>(this)->get_buffer();
        }

        bool on_key_press_event(GdkEventKey*);

     #if (GTKSVMM_API_VERSION >= 2)
        //
        // Gtkmm 2 has dropped marker support. 
        //
        void set_show_line_markers(bool show)
        {
            gtk_source_view_set_show_line_marks(GTK_SOURCE_VIEW(gobj()), show);
        }

        void set_marker_pixbuf(const gchar* category, Glib::RefPtr<Gdk::Pixbuf> p)
        {
            gtk_source_view_set_mark_category_icon_from_pixbuf(
                GTK_SOURCE_VIEW(gobj()), category, p->gobj());
        }
    #endif

    private:
        Glib::RefPtr<SourceLanguageManager> mgr_;
    };
 }
#endif
#endif // TEXT_H__6A4228B5_6E98_4B40_A53E_3F06B2F12682
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
