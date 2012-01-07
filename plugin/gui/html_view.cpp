//
// $Id: html_view.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "config.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>
#include <deque>
#include <vector>
#include "dharma/environ.h"
#include "dharma/exec.h"
#include "generic/singleton.h"
#include "gtkmm/connect.h"
#include "gtkmm/container.h"
#include "gtkmm/flags.h"
#include "zdk/export.h"


#ifdef USE_GTK_XmHTML
////////////////////////////////////////////////////////////////
 extern "C"
 {
    #include <gtk-xmhtml/gtk-xmhtml.h>
 }
 #define gtk_html_new gtk_xmhtml_new
 #define gtk_html_get_type gtk_xmhtml_get_type

////////////////////////////////////////////////////////////////
#elif GTKMM_2
extern "C"
{
#ifdef HAVE_LIBGTKHTML
 #include <gtkhtml/gtkhtml.h>
#endif
}
#include <gtk/gtkscrolledwindow.h>
#else
////////////////////////////////////////////////////////////////
// GtkHTML
 extern "C"
 {
    #include "gtkhtml/gtkhtml.h"
    #include "gtkhtml/gtkhtml-stream.h"
 }
#endif // USE_GTK_XmHTML

#include "set_cursor.h"
#include "generic/auto_file.h"
#include "html_view.h"

using namespace std;


void HTMLView::make_hrefs(string& src)
{
    size_t pos = 0;
    size_t n = src.find("http://", pos);

    while (n != string::npos)
    {
        size_t len = src.find_first_of(" ()\n\t", n);
        if (len == string::npos)
        {
            len = src.length();
        }
        len -= n;
        string href = "<a href=\"";
        const string url = src.substr(n, len);
        href += url;
        href += "\">";
        href += url;
        href += "</a>";
        src.erase(n, len);
        src.insert(n, href);
        pos = n + href.size();
        n = src.find("http://", pos);
    }
}


/**
 * Helper class for calling waitpid() on forked browser processes,
 * @see open_url_in_browser()
 */
class ZDK_LOCAL Reaper
{
    vector<pid_t> pids_;

protected:
    Reaper() { }

    ~Reaper()
    {
        while (!pids_.empty())
        {
            int status = 0;
            ::waitpid(pids_.back(), &status, 0);
        #ifdef DEBUG
            clog << __func__ << ": " << pids_.back() << endl;
        #endif
            pids_.pop_back();
        }
    }

public:
    void push_back(pid_t pid) { pids_.push_back(pid); }
};


typedef Singleton<Reaper> TheReaper;


/**
 * Launch URL in external browser
 */
bool open_url_in_browser(const string& browser, const string& url)
{
    deque<string> args;

    args.push_back(browser);
    args.push_back(url);

    try
    {
        pid_t pid = exec(browser, args);
        usleep(500);

        int status = 0;

        if (waitpid(pid, &status, WNOHANG) <= 0)
        {
            TheReaper::instance().push_back(pid);
        }
        if (status == 0)
        {
            return true;
        }
    }
    catch (const exception& e)
    {
#ifdef DEBUG
        cerr << __func__ << ": " << e.what() << endl;
#endif
    }
    return false;
}


bool open_url_in_browser(const string& url)
{
    if (url.find("http://") == 0 || url.find("mailto:") == 0)
    {
        const string browser = env::get("ZERO_BROWSER", "x-www-browser");

        if (open_url_in_browser(browser, url)
         || open_url_in_browser("firefox", url)
         || open_url_in_browser("epiphany", url)
         || open_url_in_browser("mozilla", url))
        {
            return true;
        }
    }
    return false;
}


#ifdef GTKMM_2

HTMLView::HTMLView()
    : hClick_(0)
    , hVisited_(0)
    , gobj_(GTK_HTML(gtk_html_new()))
{
    init();
    set_dynamic();
    //
    // the html widget is the child of a scrolled window
    //
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    add(*Glib::wrap(GTK_WIDGET(gobj_)));

    set_name("HTMLView");
    Gtk_SIGNAL_CONNECT(GTK_OBJECT(gtkobj()), "on_url", GtkSignalFunc(on_url), this);
}


void HTMLView::on_size_request(Gtk::Requisition* req)
{
    Gtk::PolicyType h = Gtk::POLICY_NEVER, v = Gtk::POLICY_NEVER;
    get_policy(h, v);
    if ((h == Gtk::POLICY_NEVER) && (v == Gtk::POLICY_NEVER))
    {
        gtk_widget_size_request(GTK_WIDGET(gobj_), req);
    }
    else
    {
        ScrolledWindow::on_size_request(req);
    }
}


void HTMLView::on_url(GtkWidget* w, const gchar* url, HTMLView* view)
{
    if (url)
    {
        //todo: tooltip?
    }
}
#else
//
// OLD GTK 1.2
//
HTMLView::HTMLView()
    : Gtk::Widget(GTK_WIDGET(gtk_html_new()))
    , hClick_(0)
    , hVisited_(0)
{
    init();
    set_dynamic();

 /* gtk_xmhtml_set_font_familty(gtkobj(),
        "adobe-times-normal-*", "10,8,18,16,16,12,12,8"); */
}
#endif


HTMLView::~HTMLView()
{
}


GtkType HTMLView::get_type()
{
    return gtk_html_get_type();
}


////////////////////////////////////////////////////////////////
#if USE_GTK_XmHTML
static int
_visited(GtkWidget* w, const char* anchor, void*, HTMLView* view)
{
    return view ? view->is_visited(anchor) : false;
}

GtkXmHTML* HTMLView::gtkobj()
{
    return GTK_XMHTML(this->gtkobject);
}

const GtkXmHTML* HTMLView::gtkobj() const
{
    return GTK_XMHTML(this->gtkobject);
}


void HTMLView::init()
{
    assert(hClick_ == 0 && hVisited_ == 0);

    gtk_xmhtml_set_allow_images(gtkobj(), true);
 // gtk_xmhtml_set_allow_font_switching(gtkobj(), false);
    gtk_xmhtml_set_allow_body_colors(gtkobj(), true);
    gtk_xmhtml_set_bad_html_warnings(gtkobj(), true);
 // gtk_xmhtml_set_strict_checking(gtkobj(), true);
    gtk_xmhtml_set_anchor_buttons(gtkobj(), 0);

    hClick_ = gtk_signal_connect(   GTK_OBJECT(gtkobj()),
                                    "activate",
                                    (GtkSignalFunc)click,
                                    this);
    hVisited_ = gtk_signal_connect( GTK_OBJECT(gtkobj()),
                                    "anchor-visited",
                                    (GtkSignalFunc)_visited,
                                    this);
}


void HTMLView::source(const string& src)
{
    gtk_xmhtml_source(gtkobj(), const_cast<char*>(src.c_str()));
    gtk_xmhtml_set_topline(gtkobj(), 0);
}


void HTMLView::set_base(const string&)
{
}


void HTMLView::click(GtkWidget* w, void* data, HTMLView* view)
{
    assert(GTK_IS_WIDGET(w));
    assert(data);
    assert(view);

    XmHTMLAnchorCallbackStruct* cbs = (XmHTMLAnchorCallbackStruct*)data;
    assert(cbs->href);
    if (cbs->href)
    {
        string url = cbs->href;
        view->on_click(url);
    }
}


void HTMLView::on_click(string url)
{
    if (url == "close")
    {
        if (view)
        {
            gtk_signal_disconnect(GTK_OBJECT(gtkobj()), hClick_);
            gtk_signal_disconnect(GTK_OBJECT(gtkobj()), hVisited_);
            set_cursor(*this, Gdk_FLAG(TOP_LEFT_ARROW));
            destroy_when_idle(this);
        }
    }
    else
    {
        if (open_url_in_browser(url))
        {
            return;
        }
        string anchor;
        size_t n = url.find('#');

        if (n != string::npos)
        {
            anchor = url.substr(n);
            url[n] = 0;
        }
        if (n && !url.empty())
        {
            string content = link(url.c_str());
            if (content.empty())
            {
                return;
            }
            source(content);
        }
        if (n != string::npos)
        {
            url[n] = '#';
            // note: cbs->href is no longer valid because
            // we called source()
            link(url.c_str());
            XmHTMLAnchorScrollToName(gtkobj(), &anchor[0]);
        }
    }
}

////////////////////////////////////////////////////////////////
#else /* gtk_html */

GtkHTML* HTMLView::gtkobj()
{
#if GTKMM_2
    return gobj_;
#else
    return GTK_HTML(GTKOBJ(this));
#endif
}

const GtkHTML* HTMLView::gtkobj() const
{
#if GTKMM_2
    return gobj_;
#else
    return GTK_HTML(GTKOBJ(this));
#endif
}


static void
url_request(GtkHTML* html, const gchar* url, GtkHTMLStream* handle)
{
    vector<char> buf(1024);

    auto_fd fd(open(url, O_RDONLY));
    if (fd.is_valid())
    {
        for (;;)
        {
            int rc = read(fd.get(), &buf[0], buf.size());
            if (rc == 0)
            {
                break;
            }
            if (rc < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                break;
            }
            gtk_html_write(html, handle, &buf[0], rc);
        }
    }
    gtk_html_end(html, handle, GTK_HTML_STREAM_OK);
}


void HTMLView::click(GtkWidget* w, void* data, HTMLView* view)
{
    if (data && view)
    {
        try
        {
            string url = reinterpret_cast<const char*>(data);
            view->on_click(url);
        }
        catch (const exception& e)
        {
            cerr << e.what() << endl;
        }
    }
}



void HTMLView::on_click(string url)
{
    if (url == "close")
    {
        destroy_when_idle(this);
        set_cursor(get_window(), Gdk_FLAG(TOP_LEFT_ARROW));
    }
    else
    {
        string anchor;
        size_t n = url.find('#');

        if (open_url_in_browser(url))
        {
            return;
        }
        if (n != string::npos)
        {
            anchor = url.substr(n);
            url[n] = 0;
        }
        if (n && !url.empty())
        {
            string content = link(url.c_str());
            if (content.empty())
            {
                return;
            }
            source(content);
        }
        if (n != string::npos)
        {
            url[n] = '#';
            link(url.c_str());
        #ifdef DEBUG
            clog << __func__ << ": url=" << url << endl;
            clog << __func__ << ": anchor=\"" << anchor << "\"\n";
        #endif
            gtk_html_jump_to_anchor(gtkobj(), anchor.c_str());
        }
    }
}


void HTMLView::source(const string& src)
{
    gtk_html_load_from_string(gtkobj(), src.c_str(), src.length());
    gtk_html_flush(gtkobj());
}


void HTMLView::set_base(const string& url)
{
    gtk_html_set_base(gtkobj(), url.c_str());
}



void HTMLView::init()
{
    hClick_ = Gtk_SIGNAL_CONNECT( GTK_OBJECT(gtkobj()),
                        "link_clicked",
                        (GtkSignalFunc)click,
                        this);

    Gtk_SIGNAL_CONNECT( GTK_OBJECT(gtkobj()),
                        "url_requested",
                        (GtkSignalFunc)url_request,
                        this);
}


#endif /* gtkhtml */
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
