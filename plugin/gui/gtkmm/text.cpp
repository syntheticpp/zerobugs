// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "config.h"
#include <assert.h>
#ifdef DEBUG
 #include <iostream>
#endif
#include "text.h"
#include <gtksourceview/gtksourceview.h>
#if GTKSVMM_API_VERSION >= 2
 #include <gtksourceviewmm/sourcelanguagemanager.h>
 #include <gtksourceview/gtksourcelanguagemanager.h>
#else
#include <gtksourceview/gtksourcelanguagesmanager.h>
 #include <gtksourceviewmm/sourcelanguagesmanager.h>
#endif

// C/C++ delimiters
static const std::string cpp_delim(".,;+-/*%|&~^=<>()[]{}!\"' \t\n");

using namespace std;
using namespace Gtk;


////////////////////////////////////////////////////////////////
static Glib::ustring to_string(const Gdk_Color& color)
{
    char buf[32];
    snprintf(buf, sizeof buf, "#%04x%04x%04x", color.get_red(),
        color.get_green(), color.get_blue());

    // this does not work if the color is not allocated
    //snprintf(buf, sizeof buf, "%04x", color.get_pixel());
    return buf;
}


////////////////////////////////////////////////////////////////
Text::Text()

#ifdef HAVE_SOURCE_LANGUAGES_MANAGER_CREATE
    : mgr_(Gtk::SourceLanguageManager::create())
#else
    : mgr_(new Gtk::SourceLanguageManager)
#endif
{
    set_cursor_visible(false);
    //set_insert_spaces_instead_of_tabs();
}


Text::~Text()
{
}


////////////////////////////////////////////////////////////////
void Text::set_language(const char* name)
{
    Glib::RefPtr<SourceBuffer> buf = get_buffer();
    if (buf)
    {
        Glib::ustring mimeType = "text/x-";
        mimeType += name;

        assert(mgr_);

        if (mgr_)
        {
            GtkSourceLanguage* lang = NULL;
        #if GTKSVMM_API_VERSION >= 2
            if (strcmp(name, "c++") == 0)
            {
                name = "cpp";
            }
            lang = gtk_source_language_manager_get_language(mgr_->gobj(), name);
        #else
            lang = gtk_source_languages_manager_get_language_from_mime_type(
                    mgr_->gobj(),
                    mimeType.c_str());
            if (!lang)
            {
                mimeType += "src";
                lang = gtk_source_languages_manager_get_language_from_mime_type(
                    mgr_->gobj(),
                    mimeType.c_str());
            }
            buf->set_highlight(true);
        #endif
            if (lang)
            {
                gtk_source_buffer_set_language(buf->gobj(), lang);
            }
            //buf->set_highlight(true);
        }
    }
}


////////////////////////////////////////////////////////////////
size_t Text::get_length() const
{
    Glib::RefPtr<const SourceBuffer> buf = get_buffer();
    if (buf)
    {
        return buf->size();
    }
    return 0;
}

////////////////////////////////////////////////////////////////
string Text::get_chars(int from, int to) const
{
    string text;
    Glib::RefPtr<SourceBuffer> buf = const_cast<Text*>(this)->get_buffer();
    if (buf)
    {
        SourceBuffer::iterator begin = buf->get_iter_at_offset(from);

        SourceBuffer::iterator end = (to == -1)
            ? buf->end() : buf->get_iter_at_offset(to);

        text = buf->get_text(begin, end).raw();
    }
    return text;
}

////////////////////////////////////////////////////////////////
void Text::set_point(guint pos)
{
    Glib::RefPtr<SourceBuffer> buf = get_buffer();
    if (buf)
    {
        SourceBuffer::iterator iter = buf->get_iter_at_offset(pos);
        buf->place_cursor(iter);
    }
}


////////////////////////////////////////////////////////////////
Gtk::SourceBuffer::iterator
Text::insert(const Pango::FontDescription& font,
             const Glib::ustring& text)
{
    Gtk::SourceBuffer::iterator iter;

    if (Glib::RefPtr<SourceBuffer> buf = get_buffer())
    {
        iter = buf->end();

        if (Glib::RefPtr<SourceBuffer::Mark> mark = buf->get_insert())
        {
            iter = buf->get_iter_at_mark(mark);
        }
        Glib::RefPtr<Gtk::SourceBuffer::TagTable> tagTable =
            buf->get_tag_table();

        if (tagTable)
        {
            Glib::RefPtr<Gtk::SourceBuffer::Tag> tagFont =
                tagTable->lookup(font.to_string());

            if (!tagFont)
            {
                tagFont =
                    Gtk::SourceBuffer::Tag::create(font.to_string());
                tagFont->property_font_desc() = font;

                tagTable->add(tagFont);
            }

            iter = buf->insert_with_tag(iter, text, tagFont);
        }
        else
        {
            buf->insert_at_cursor(text);
            iter = buf->end();
        }
    }
    return iter;
}


////////////////////////////////////////////////////////////////
void Text::insert(Context& c, const Glib::ustring& text)
{
    Glib::RefPtr<SourceBuffer> buf = get_buffer();
    if (buf)
    {
        NotUndoableScope notUndoable(buf);
        Gtk::SourceBuffer::iterator iter = buf->end();
        Glib::RefPtr<SourceBuffer::Mark> mark = buf->get_insert();
        if (mark)
        {
            iter = buf->get_iter_at_mark(mark);
        }

        Glib::RefPtr<Gtk::SourceBuffer::TagTable> tagTable =
            buf->get_tag_table();

        if (tagTable)
        {
            Glib::ustring name;
            if (c.bgSet_)
            {
                name += to_string(c.bg_);
            }
            if (c.fgSet_)
            {
                name += to_string(c.fg_);
        #ifdef DEBUG
                //clog << __func__ << ": tag=" << name << endl;
        #endif
            }
            if (c.fontSet_)
            {
                name += c.font_.to_string();
            }
            Glib::RefPtr<Gtk::SourceBuffer::Tag> tag =
                tagTable->lookup(name);
            if (!tag)
            {
                tag = Gtk::SourceBuffer::Tag::create(name);
                tagTable->add(tag);
                if (c.fontSet_)
                {
                    tag->property_font_desc() = c.font_;
                }
                if (c.fgSet_)
                {
                    tag->property_foreground_gdk() = c.fg_;
                }
                if (c.bgSet_)
                {
                    tag->property_background_gdk() = c.bg_;
                }
            }
            buf->insert_with_tag(iter, text, tag);
            c.fgSet_ = false;
            c.bgSet_ = false;
        }
        else
        {
        #ifdef DEBUG
            clog << __func__ << ": no tagTable\n";
        #endif
            buf->insert_at_cursor(text);
        }
    }
}


////////////////////////////////////////////////////////////////
void Text::insert(
    const Pango::FontDescription& font,
    const Gdk_Color& foregnd,
    const Gdk_Color& backgnd,
    const string& s,
    int pos) // position (offset) where to insert
{
    Glib::ustring text(s);

    Glib::RefPtr<SourceBuffer> buf = get_buffer();
    if (buf)
    {
        // convert insertion position to iterator
        SourceBuffer::iterator iter;
        if (pos == -1)
        {
            Glib::RefPtr<SourceBuffer::Mark> mark = buf->get_insert();
            if (mark)
            {
                iter = buf->get_iter_at_mark(mark);
            }
        }
        else
        {
            iter = buf->get_iter_at_offset(pos);
        }

        Glib::ustring name =
            to_string(backgnd) + to_string(foregnd) + font.to_string();

        Glib::RefPtr<SourceBuffer::TagTable> table = buf->get_tag_table();
        Glib::RefPtr<Gtk::SourceBuffer::Tag> tag = table->lookup(name);
        if (!tag)
        {
            tag = Gtk::SourceBuffer::Tag::create();
            tag->property_background_gdk() = backgnd;
            tag->property_foreground_gdk() = foregnd;
            tag->property_font_desc() = font;

            table->add(tag);
        }

        buf->insert_with_tag(iter, text, tag);
    }
}


////////////////////////////////////////////////////////////////
bool Text::forward_delete(guint nchars)
{
    Glib::RefPtr<SourceBuffer> buf = get_buffer();
    if (buf)
    {
        Glib::RefPtr<SourceBuffer::Mark> mark = buf->get_insert();
        if (mark)
        {
            SourceBuffer::iterator begin = buf->get_iter_at_mark(mark);

            if (!begin)
            {
                return false;
            }
            SourceBuffer::iterator end = begin;
            end.forward_chars(nchars);

            buf->erase(begin, end);
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
bool Text::delete_text(gint first, gint last)
{
    Glib::RefPtr<SourceBuffer> buf = get_buffer();
    if (buf)
    {
        SourceBuffer::iterator begin = buf->get_iter_at_offset(first);
        if (!begin)
        {
            assert(buf->size() == 0);
            return false;
        }
        SourceBuffer::iterator end =
            (last == -1) ? buf->end() : buf->get_iter_at_offset(last);

        buf->erase(begin, end);
        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////
gint Text::get_position() const
{
    Glib::RefPtr<SourceBuffer> buf = const_cast<Text*>(this)->get_buffer();
    if (buf)
    {
        Glib::RefPtr<SourceBuffer::Mark> mark = buf->get_insert();
        if (mark)
        {
            SourceBuffer::iterator iter = buf->get_iter_at_mark(mark);
            return iter.get_offset();
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////
void Text::select_region(int startPos, int endPos)
{
    Glib::RefPtr<SourceBuffer> buf = this->get_buffer();
    if (buf)
    {
        SourceBuffer::iterator begin = buf->get_iter_at_offset(startPos);
        SourceBuffer::iterator end = (endPos == -1)
            ? buf->end() : buf->get_iter_at_offset(endPos);

        buf->select_range(begin, end);
    }
}


////////////////////////////////////////////////////////////////
bool Text::has_selection() const
{
    Glib::RefPtr<const SourceBuffer> buf = this->get_buffer();
    if (buf)
    {
        Gtk::SourceBuffer::iterator begin, end;
        return buf->get_selection_bounds(begin, end);
    }
    return false;
}


////////////////////////////////////////////////////////////////
size_t Text::get_selection_start_pos() const
{
    Glib::RefPtr<const SourceBuffer> buf = this->get_buffer();
    if (buf)
    {
        Gtk::SourceBuffer::iterator begin, end;
        if (buf->get_selection_bounds(begin, end))
        {
            return begin.get_offset();
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////
size_t Text::get_selection_end_pos() const
{
    Glib::RefPtr<const SourceBuffer> buf = this->get_buffer();
    if (buf)
    {
        Gtk::SourceBuffer::iterator begin, end;
        if (buf->get_selection_bounds(begin, end))
        {
            return end.get_offset();
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////
string Text::get_word_at_iter(Gtk::TextBuffer::iterator iter)
{
    string word;

    Glib::RefPtr<Gtk::TextBuffer> buf = get_buffer();
    if (iter && buf)
    {
        Gtk::TextBuffer::iterator first = iter;
        Gtk::TextBuffer::iterator last = iter;

        if (backward_display_line(first))
        {
            ++first;
        }
        forward_display_line(last);

        // check if text is selected:
        Gtk::TextBuffer::iterator selBegin = buf->end();
        Gtk::TextBuffer::iterator selEnd = buf->end();
        if (buf->get_selection_bounds(selBegin, selEnd) && selBegin < iter /* && iter >= selEnd */)
        {
            word = buf->get_text(selBegin, selEnd);
        }
        else
        {
            string part1 = buf->get_text(first, iter);
            string part2 = buf->get_text(iter, last);

            size_t n = part1.find_last_of(cpp_delim);
            ++n;

            size_t m = part2.find_first_of(cpp_delim);
            if (m < part2.size())
            {
                m += part1.size() - n;
            }
            word = (part1 + part2).substr(n, m);
        }
    /* #if DEBUG
        clog << __func__ << ": " << word << endl;
    #endif */
    }
    return word;
}


////////////////////////////////////////////////////////////////
int Text::scroll(size_t line)
{
    if (!gobj()->parent.onscreen_validated)
    {
        return 1;
    }
    Glib::RefPtr<Gtk::TextBuffer> buf = get_buffer();
    if (buf)
    {
        Gtk::TextBuffer::iterator iter =
            buf->get_iter_at_line(line);

        if (iter)
        {
            if (!scroll_to(iter, .1))
            {
                Glib::RefPtr<TextBuffer::Mark> mark =
                    buf->create_mark(iter);

                scroll_to(mark, .1);
                buf->delete_mark(mark);
            }
        }
    #if DEBUG
        else
        {
            clog << __func__ << ": no iter at " << line << endl;
        }
    #endif
    }
    return 0;
}


////////////////////////////////////////////////////////////////
//
// work around Gtk (or GtkSourceView?) bug, that lets the Tab
// key insert a tab event when editable is set to false
//
bool Text::on_key_press_event(GdkEventKey* event)
{
    if (event && event->keyval == GDK_Tab)
    {
        event->keyval = GDK_bar;
    }
    return TextView::on_key_press_event(event);
}


////////////////////////////////////////////////////////////////
Glib::RefPtr<SourceBuffer> Text::get_buffer()
{
    return Glib::RefPtr<SourceBuffer>::cast_static(SourceView::get_buffer());
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
