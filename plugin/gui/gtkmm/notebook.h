#ifndef NOTEBOOK_H__F454C458_A066_455B_B6A5_92C531216DDC
#define NOTEBOOK_H__F454C458_A066_455B_B6A5_92C531216DDC
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

#include "config.h"
#include "generic/adapt_ptr.h"
#include "zdk/check_ptr.h"
#include "zdk/export.h"

#ifdef GTKMM_2
 #include <gtkmm/notebook.h>
 #define Gtk_NOTEBOOK_PAGE GtkNotebookPage


 struct ZDK_LOCAL CustomNotebook : public Gtk::Notebook
 {
    CustomNotebook();

    bool on_map_event(GdkEventAny*);
    bool on_expose_event(GdkEventExpose*);
    void on_style_changed(const Glib::RefPtr<Gtk::Style>& previous);

    void set_tab_detachable(Gtk::Widget&) { }
#if !defined (HAVE_EXTRA_UI_FEATURES) || !defined (HAVE_NOTEBOOK_SET_GROUP)
    void set_group(void*) { }
    void set_tab_reorderable(Gtk::Widget&) { }
#endif // HAVE_EXTRA_UI_FEATURES

    int append_page(Gtk::Widget& w, const Glib::ustring&);
    void set_tab_label_text(Gtk::Widget&, const Glib::ustring&);

private:
    void set_custom_style();
    void on_size_allocate(Gtk::Allocation&);
    void on_size_request(Gtk::Requisition*);
 };

 typedef CustomNotebook Notebook_Adapt;
 typedef boost::shared_ptr<CustomNotebook> NotebookPtr;

#define get_page(n) pages()[(n)]
#else
//
// gtk-- 1.2
//
 #include <gtk--/notebook.h>
 #define Gtk_NOTEBOOK_PAGE Gtk::Notebook::Page

 /**
  * Adapt the old gtk 1.2 notebook page class by augmenting its
  * interface with 2.x-like methods, as expected by the app
  */
 struct NotebookPage_Adapt : public Gtk::Notebook_Helpers::Page
 {
    NotebookPage_Adapt() : Gtk::Notebook_Helpers::Page(0)
    {
    }

    inline void set_tab_label_text(const char* text)
    {
        set_tab_text(text);
    }

    inline void set_tab_label(Gtk::Widget& w)
    {
        set_tab(w);
    }

    void set_menu_label_text(const char* text)
    {
        Gtk::Label* label = manage(new Gtk::Label(text));
        set_menu(*label);
    }
 };


 /**
  * Adapt the old gtk 1.2 notebook class by augmenting its
  * interface with 2.x-like methods, as expected by the app
  */
 struct Notebook_Adapt : public Gtk::Notebook
 {
    void append_page(Gtk::Widget& w, const Gtk::nstring& text)
    {
        this->add(w);
        this->pages().back()->set_tab_text(text);
    }

    inline gint get_current_page() const
    {
        gint n = this->get_current_page_num();

        return n;
    }

    void set_current_page(int n)
    {
        this->set_page(n);
    }
    NotebookPage_Adapt& get_page(size_t n)
    {
        return static_cast<NotebookPage_Adapt&>(*CHKPTR(pages()[n]));
    }

    virtual void on_switch_page(Gtk_NOTEBOOK_PAGE* p, guint n)
    {
        Gtk::Notebook::switch_page_impl(p, n);
    }
    virtual void switch_page_impl(Gtk_NOTEBOOK_PAGE* p, guint n)
    {
        on_switch_page(p, n);
    }

    size_t get_n_pages() const { return pages().size(); }

    void remove_page(size_t pageIndex)
    {
        gtk_notebook_remove_page(gtkobj(), pageIndex);
    }

    void set_tab_label_text(const Gtk::Widget& child, const char* text)
    {
        int pageNum = page_num(child);
        if (pageNum >= 0)
        {
            pages()[pageNum]->set_tab_text(text);
        }
    }
    void set_group(void*) { }
    void set_tab_detachable(Gtk::Widget&) { }
    void set_tab_reorderable(Gtk::Widget&) { }
 };

 typedef boost::shared_ptr<Notebook_Adapt> NotebookPtr;

#endif // gtk-- 1.2
#endif // NOTEBOOK_H__F454C458_A066_455B_B6A5_92C531216DDC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
