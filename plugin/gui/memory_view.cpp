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
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <boost/format.hpp>
#include <gdk/gdkkeysyms.h>
#include "zdk/variant.h"
#include "generic/temporary.h"
#include "gtkmm/accelgroup.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/connect.h"
#include "gtkmm/entry.h"
#include "gtkmm/flags.h"
#include "gtkmm/font.h"
#include "gtkmm/label.h"
#include "gtkmm/resize.h"
#include "gtkmm/style.h"
#include "gtkmm/text.h"
#include "gtkmm/widget.h" // for get_width() get_height()
#include "ensure_font.h"
#include "eval_events.h"
#include "fixed_font.h"
#include "highlight_changes.h"
#include "memory_view.h"
#include "scope_helpers.h"
#include "text_entry.h"


using namespace std;


MemoryView::MemoryView(Properties* props, RefPtr<Thread> thread, addr_t addr)
    : OnMapEventImpl<Gtk::Window>(Gtk_FLAG(WINDOW_TOPLEVEL))
    , thread_(thread)
    , addr_(addr)
    , entry_(0)
    , text_(0)
    , btn_(0)
    , font_(fixed_font())
    , frozen_(false)
    , pending_(false)
    , events_(EvalEvents::create(thread.get()))
{
    Gtk_set_size(this, 480, 300);

    //
    // expression-evaluation signals
    //
    events_->signal_done()->connect(Gtk_SLOT(this, &MemoryView::on_done));
    events_->signal_error()->connect(Gtk_SLOT(this, &MemoryView::on_error));
    events_->signal_warning()->connect(Gtk_SLOT(this, &MemoryView::on_error));

    Gtk::Box* vbox = manage(new Gtk::VBox());
    add(*vbox);
    vbox->set_spacing(2);
    vbox->set_border_width(2);

    Gtk::Box* hbox = manage(new Gtk::HBox());
    vbox->pack_start(*hbox, false, false);
    hbox->set_spacing(3);

    Gtk::Label* label = manage(new Gtk::Label());
    label->set_text("Address: ");
    hbox->pack_start(*label, false, false);

    entry_ = manage(new TextEntry(props, "memory_view"));
    Gtk_set_size(entry_, 200, -1);

    hbox->pack_start(*entry_, false, true);
    Gtk_CONNECT_0(entry_, activate, this, &MemoryView::activate);

    const string name = "Freeze _View";
    btn_ = manage(new Gtk::Button(name));
    hbox->pack_start(*btn_, false, true);
    btn_->set_border_width(2);
    Gtk_CONNECT_0(btn_, clicked, this, &MemoryView::toggle_freeze);
    text_ = manage(new Gtk::Text());
    CHKPTR(text_)->set_line_wrap(false);
    text_->set_name("MemoryView");
    text_->set_editable(false);
    vbox->add(*text_);

    ostringstream title;
    title << "zero: memory view";
    if (thread_.get())
    {
        title << " [" << thread_->lwpid() << ']';
    }
    set_title(title.str());

    if (Gtk_ACCEL_GROUP_PTR accelGroup = get_accel_group())
    {
        using Gtk::Label;

        int accel = 0;
        if (Label* label = dynamic_cast<Label*>(btn_->get_child()))
        {
            accel = Gtk_get_mnemonic_keyval(label);
        }
        if (accel != GDK_VoidSymbol)
        {
            Gtk_ADD_ACCEL(*btn_, "clicked", accelGroup, accel,
                Gdk_FLAG(MOD1_MASK), Gtk_FLAG(ACCEL_LOCKED));
        }
    }

    ensure_monospace(font_, *CHKPTR(text_));
}


MemoryView::~MemoryView() throw()
{
    events_->deactivate();
}


#ifdef GTKMM_2
/**
 * Re-read memory when the window is resized
 */
void MemoryView::on_size_allocate(Gtk::Allocation& allocation)
{
    Gtk::Window::on_size_allocate(allocation);

    if (rect_.get_x() != allocation.get_x()
        || rect_.get_y() != allocation.get_y()
        || rect_.get_width() != allocation.get_width()
        || rect_.get_height() != allocation.get_height())
    {
        update_impl(true);
    }
    rect_ = allocation;
}

#else // gtk-- 1.2

/**
 * Re-read memory when the window is resized
 */
void MemoryView::size_allocate_impl(GtkAllocation* pa)
{
    Gtk::Window::size_allocate_impl(pa);
    update_impl(true);
}

#endif // gtk-- 1.2


event_result_t MemoryView::delete_event_impl(GdkEventAny*)
{
    destroy_();
    return 0;
}


void MemoryView::activate()
{
    events_->activate();
    if (is_realized())
    {
        set_cursor(*this, Gdk_FLAG(WATCH));
    }
    evaluate(entry_->get_text(), 0, events_.get(), 0);
}


void MemoryView::get_geometry(int& nrows, int& ncols)
{
#ifdef GTKMM_2
    Glib::RefPtr<Pango::Context> ctxt = CHKPTR(text_)->get_pango_context();

    Pango::FontMetrics fm =
        ctxt->get_metrics(font_, Pango::Language("common"));

    double w = fm.get_approximate_digit_width();
    double z = fm.get_approximate_char_width();
    double height = fm.get_ascent() + fm.get_descent();

#ifdef DEBUG
    assert(w && z && height);
#endif

    w = (w + z) / 2;
    w = PANGO_PIXELS(w);
    z = PANGO_PIXELS(z);
    height = PANGO_PIXELS(height);

    if (!w || !z)
    {
        w = z = 8;
        height = 15;
    }

    nrows = static_cast<int>(text_->get_height() / height);
    ncols = static_cast<int>(
        (text_->get_width() - (sizeof(addr_t) + 2) * 2 * w - z)
            / (3 * w + z));
#else
    const int w = font_.char_width('0');
    const int z = font_.char_width(' ');

    assert(font_.height());

    nrows = text_->height() / font_.height();
    ncols = (text_->width() - 4 * z - 9 * w) / (3 * w + z);
#endif

    assert(nrows > 0);
    assert(ncols > 0);
}


void MemoryView::update_impl(bool request)
{
    int rows = 0, cols = 0;
    get_geometry(rows, cols);

    if (request && addr_ && thread_.get())
    {
        data_.resize(rows * cols / sizeof(long));
        pending_ = true;

        // the main window will bounce back an
        // update() call on the main thread
        update_request(this);
    }
    else
    {
        display_impl(rows, cols);
    }
}


void MemoryView::display_impl(int rows, int cols)
{
    set_cursor(*this, Gdk_FLAG(TOP_LEFT_ARROW));

    if (pending_)
    {
        return;
    }

    assert(rows);
    assert(cols);

    Gtk::Text::Context context = text_->get_context();
    const Gdk_Color fg("black");
    const Gdk_Color hilite(highlight_changes_color());
    context.set_font(font_);
    context.set_foreground(fg);

    char buf[32] = { 0 };

    string addrFormat =
        (boost::format("0x%%0%1%lx") % (sizeof(addr_t) * 2)).str();

    ScopedFreeze<Gtk::Text> freeze(*text_);
    text_->delete_text(0, -1);

    // todo: use boost::format
    snprintf(buf, sizeof(buf) - 1, addrFormat.c_str(), addr_);
    entry_->set_text(buf);

    assert(rows * cols / sizeof(long));

    data_.resize(rows * cols / sizeof(long));
    assert(!data_.empty());

    const uint8_t* bytes = reinterpret_cast<uint8_t*>(&data_[0]);
    assert(bytes);
    const int dataLen = static_cast<int>(data_.size() * sizeof(long));

    if (oldData_.empty())
    {
        oldData_ = data_;
    }

    const int oldDataLen = oldData_.size() * sizeof(long);
    const uint8_t* oldBytes = reinterpret_cast<uint8_t*>(&oldData_[0]);

    for (int i = 0; i != rows; ++i)
    {
        if (i)
        {
            text_->insert(font_, "\n");
        }
        // todo: use boost::format instead of snprintf
        snprintf(buf, sizeof(buf), addrFormat.c_str(), addr_ + i * cols);
        text_->insert(font_, buf);
        text_->insert(font_, "  ");

        for (int j = 0, index = i * cols; j != cols; ++j, ++index)
        {
            assert(index >= 0);
            if (index >= dataLen)
            {
                strcpy(buf, "   ");
            }
            else
            {
                snprintf(buf, sizeof(buf), "%02x ", bytes[index]);
            }
            if ((index < oldDataLen) && (bytes[index] != oldBytes[index]))
            {
                context.set_foreground(hilite);
            }
            text_->insert(context, buf);
            context.set_foreground(fg);
        }
        text_->insert(font_, " ");

        buf[1] = 0;
        for (int j = 0, index = i * cols; j != cols; ++j, ++index)
        {
            const int x = (index < dataLen) ? bytes[index] : 0;

            if (index < oldDataLen  &&
                index < dataLen     &&
                bytes[index] != oldBytes[index])
            {
                context.set_foreground(hilite);
            }
            buf[0] = isprint(x) ? x : '.';
    #ifdef GTKMM_2
            Glib::ustring line(buf);
            if (!line.validate())
            {
                buf[0] = '.';
            }
    #endif
            text_->insert(context, buf);
            context.set_foreground(fg);
        }
    }
    oldData_ = data_;
}


void MemoryView::show_all_impl()
{
    Window::show_all_impl();
    entry_->grab_focus();
    display();
}


/**
 * @note runs on main thread
 */
void MemoryView::update()
{
    if (!frozen_ && addr_ && error_.empty() && thread_.get())
    {
        try
        {
            thread_->read_data(addr_, &data_[0], data_.size());
        }
        catch (exception& e)
        {
            string tmp(e.what());

            error_ += '\n';
            error_ += tmp.substr(0, tmp.find('\n'));
        }
    }
    pending_ = false;
}


void MemoryView::display()
{
    int rows = 0, cols = 0;
    get_geometry(rows, cols);

    if (error_.empty())
    {
        display_impl(rows, cols);
    }
    else
    {
        display_impl(rows ? rows - 1 : 0, cols);

        Gdk_Color hilite(highlight_changes_color());
        Gdk_Color yellow("yellow");
        text_->insert(font_, hilite, yellow, error_, -1);

        error_.erase();
    }
    Gtk_WINDOW(this)->raise();
}


void MemoryView::toggle_freeze()
{
    assert(btn_);

    frozen_ ^= true;
    if (!frozen_)
    {
        btn_->remove();
        btn_->add_label("Freeze _View");
    }
    else
    {
        btn_->remove();
        btn_->add_label("Thaw _View");
    }
    btn_->set_border_width(2);

    using Gtk::Label;
    if (Label* label = dynamic_cast<Label*>(btn_->get_child()))
    {
        Gtk_get_mnemonic_keyval(label);
    }
}


/**
 * @note runs on main thread
 */
void MemoryView::on_done(const Variant& var)
{
    addr_t addr = 0;
    switch (var.type_tag())
    {
    case Variant::VT_ARRAY:
        if (DebugSymbol* sym = var.debug_symbol())
        {
            addr = sym->addr();
        }
        break;

    case Variant::VT_POINTER:
        addr = var.pointer();
        break;

    default:
        addr = var.bits();
        break;
    }

    if (addr != addr_)
    {
        addr_ = addr;
        if (frozen_)
        {
            toggle_freeze();
        }
        oldData_.clear();
    }
    update_impl();
}


/**
 * @note runs on main thread
 */
bool MemoryView::on_error(string msg)
{
    error_ = "\n" + msg;
    addr_ = (addr_t)-1;

    // need to disconnect the events, the window is modeless
    if (events_)
    {
        events_->deactivate();
    }
    update_impl(true);
    return true;
}


EvalEvents& MemoryView::events()
{
    assert(events_.get());
    return *events_;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
