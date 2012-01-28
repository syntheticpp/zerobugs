#ifndef HTML_VIEW_H__6A9919A5_D7F7_41B3_A1DD_935B91C1A26E
#define HTML_VIEW_H__6A9919A5_D7F7_41B3_A1DD_935B91C1A26E
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
//
#include <deque>
#include <string>
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/signal.h"
#include "gtkmm/widget.h"
#include "zdk/export.h"


#ifdef USE_GTK_XmHTML
 typedef struct _GtkXmHTML GtkHTML;
#else
 typedef struct _GtkHTML GtkHTML;
#endif

#if GTKMM_2
 typedef Gtk::ScrolledWindow HTMLViewBase;
#else
 typedef Gtk::Widget HTMLViewBase;
#endif


CLASS HTMLView : public HTMLViewBase
{
public:
    HTMLView();

    virtual ~HTMLView();

    static void make_hrefs(std::string&);

    void source(const std::string&);

    void set_base(const std::string&);

    static GtkType get_type();

    SigC::Signal1<std::string, const char*> link;

    SigC::Signal1<bool, const char*> is_visited;

    SigC::Signal1<void, Gtk::Widget*> destroy_when_idle;

#if GTKMM_2
    void set_dynamic() {}

    SigC::Signal1<std::string, const char*>& signal_link()
    { return link; }

    SigC::Signal1<bool, const char*>& signal_is_visited()
    { return is_visited; }

    SigC::Signal1<void, Gtk::Widget*>& signal_destroy_when_idle()
    { return destroy_when_idle; }

    void on_size_request(Gtk::Requisition*);
    static void on_url(GtkWidget*, const gchar*, HTMLView*);
#endif // GTKMM_2

protected:
    explicit HTMLView(GtkHTML*);

    void init();

    static void click(GtkWidget*, void*, HTMLView*);
    virtual void on_click(std::string url);

    GtkHTML* gtkobj();
    const GtkHTML* gtkobj() const;

private:
    HTMLView(const HTMLView&);
    HTMLView& operator=(const HTMLView&);

    guint hClick_, hVisited_;

#if GTKMM_2
    GtkHTML* gobj_;
#endif
};

bool open_url_in_browser(const std::string& url);
bool open_url_in_browser(const std::string& browser, const std::string& url);


#endif // HTML_VIEW_H__6A9919A5_D7F7_41B3_A1DD_935B91C1A26E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
