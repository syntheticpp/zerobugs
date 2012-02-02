#ifndef EDIT_IN_PLACE_H__26E266A5_392F_45E6_98A7_1E72516B3F45
#define EDIT_IN_PLACE_H__26E266A5_392F_45E6_98A7_1E72516B3F45
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
#include <memory>
#include <vector>
#include "gtkmm/base.h"
#include "gtkmm/ctree.h"
#include "gtkmm/signal.h"

#ifdef GTKMM_2
 #include <glibmm/sarray.h>
 #include <gtkmm/accelgroup.h>
 #include <gtkmm/window.h>

 namespace Gtk
 {
    typedef Glib::SArray SArray;
 }
#endif

namespace Gtk
{
    class CList;
    class Entry;
    class Window;
}

#if !defined(GTKMM_2)

/**
 * Adds an edit-in-place feature to CList and CTree widgets;
 * a cell can be edited by double clicking on it; the cell_edit
 * signal is emitted. The change is applied only when the
 * signal returns true.
 */
class EditInPlace : public Gtk::Base
{
public:
    explicit EditInPlace(Gtk::CList*);

    SigC::Signal4<bool, int, int, std::string, std::string&> cell_edit;

    void set_column_editable(int ncol, bool mode)
    {
        editable_[ncol] = mode;
    }

    void begin_edit(int nrow, int ncol, GdkEventButton*);

    void finish_edit();

    void on_select_row(int row, int col, GdkEvent*);

    int on_map_event(GdkEventAny*);

protected:
    /**
     * Handle the Return and Esc keys
     */
    int on_key(GdkEventKey*);

    /**
     * Aborts editing when clicked outside the edit widget.
     */
    int on_mouse_click(GdkEventButton*);

    int on_configure(GdkEventConfigure*);

    int on_visibility(GdkEventVisibility*);

    /**
     * Adjust coordinates to the upper-left corner
     * of the bounding cell.
     */
    void adjust_to_cell(int& x, int& y);

    Gtk::CList& get_list() { assert(list_); return *list_; }

    void commit_edit();

private:
    Gtk::CList* list_; /* this works because CTree inherits CList */

    bool mapped_;

    /* toplevel window that holds the Entry widget */
    std::auto_ptr<Gtk::Window> w_;

    /**
     * The entry widget, for in-place editing. The pointer
     * is valid only during editing. When not in edit mode,
     * the pointer is NULL.
     */
    Gtk::Entry* edit_;

    /* columns editable state */
    std::vector<bool> editable_;

    int nrow_, ncol_; /* the cell being edited */
};
#endif // !GTKMM_2

#ifdef GTKMM_2

#define CELL_EDIT_PARAM \
    const Gtk::TreePath& path, \
    int ncol, \
    const Glib::ustring& old, \
    const Glib::ustring& s

 /**
  * Decorator
  */
 template<typename T>
 struct ZDK_LOCAL EditableInPlace : public T
 {
    typedef SigC::Signal4<
        bool,                   // return value
        const Gtk::TreePath&,   // path
        int,                    // column
        const Glib::ustring&,   // old value
        const Glib::ustring&    // new value
    > EditSignal;

    template<typename U>
    EditableInPlace(U arg) : T(arg)
    { }

    EditSignal& signal_cell_edit() { return cellEdit_; }

    void set_column_editable(int ncol, bool mode)
    {
        if (Gtk::CellRendererText* cellRenderer =
            dynamic_cast<Gtk::CellRendererText*>(
                this->get_column_cell_renderer(ncol)))
        {
            cellRenderer->property_editable() = mode;
            if (mode)
            {
            #if (GTK_MAJOR_VERSION <= 2 && GTK_MINOR_VERSION <= 10)
                #warning CELL_RENDERER_BUG_(need_gtk_2.10_or_older)
            #else

                cellRenderer->signal_editing_started().connect(
                    sigc::mem_fun(*this, &EditableInPlace::on_edit_started));

            #endif
                cellRenderer->signal_editing_canceled().connect(
                        sigc::mem_fun(*this, &EditableInPlace::on_done));
                cellRenderer->signal_edited().connect(
                    sigc::bind(
                        sigc::mem_fun(*this, &EditableInPlace::on_edited),
                        ncol));
            }
        }
    }

    SigC::Signal0<void>& signal_edit_started() { return editStarted_; }
    SigC::Signal0<void>& signal_edit_finished() { return editFinished_; }

 private:
    EditSignal cellEdit_;
    SigC::Signal0<void> editStarted_;
    SigC::Signal0<void> editFinished_;
    Glib::RefPtr<Gtk::AccelGroup> accelGrp_;

    void on_edit_started(Gtk::CellEditable* cell, const Glib::ustring&)
    {
        if (Gtk::Window* w = dynamic_cast<Gtk::Window*>(this->get_toplevel()))
        {
            accelGrp_ = w->get_accel_group();
            if (accelGrp_)
            {
                w->remove_accel_group(accelGrp_);
            }
        }
        editStarted_();
    }

    void on_edited(const Glib::ustring& pathStr,
                   const Glib::ustring& value,
                   int ncol)
    {
        Gtk::TreePath path(pathStr);

        if (Gtk::TreeIter iter = this->get_iter(path))
        {
            std::string old = (*iter)[this->model_column(ncol)];
            if (cellEdit_(path, ncol, old, value))
            {
                (*iter)[this->model_column(ncol)] = value;
            }
        }
        on_done();
    }

    void on_done()
    {
        if (Gtk::Window* w = dynamic_cast<Gtk::Window*>(this->get_toplevel()))
        {
            if (accelGrp_)
            {
                w->add_accel_group(accelGrp_);
            }
        }
        editFinished_();
    }
 };

#else
#define CELL_EDIT_PARAM \
    int path, int ncol, std::string old, std::string& s

//
// Gtk-- 1.2 hacks
//
 template<typename T>
 class EditableInPlace : public T
 {
    typedef T base_class;

public:
    explicit EditableInPlace(const Gtk::SArray &titles)
        : T(titles)
        , impl_(this)
    {
        impl_.cell_edit.connect(cell_edit.slot());
    }

    void set_column_editable(int ncol, bool mode)
    {
        impl_.set_column_editable(ncol, mode);
    }

    SigC::Signal4<bool, int, int, std::string, std::string&> cell_edit;

protected:
    void select_row_impl(int row, int col, GdkEvent* event)
    {
        base_class::select_row_impl(row, col, event);
        impl_.on_select_row(row, col, event);
    }

    int map_event_impl(GdkEventAny* event)
    {
        int result = base_class::map_event_impl(event);
        impl_.on_map_event(event);
        return result;
    }

private:
    EditInPlace impl_;
 };
#endif
#endif // EDIT_IN_PLACE_H__26E266A5_392F_45E6_98A7_1E72516B3F45
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
