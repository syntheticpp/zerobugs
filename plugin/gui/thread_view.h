#ifndef THREAD_VIEW_H__AF353FD4_BA41_4D44_8FA1_02A7133F5B94
#define THREAD_VIEW_H__AF353FD4_BA41_4D44_8FA1_02A7133F5B94
//
// $Id: thread_view.h 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <queue>
#include <string>
#include "gtkmm/box.h"
#include "gtkmm/ctree.h"
#include "gtkmm/events.h"
#include "gtkmm/scrolledwindow.h"
#include "custom_tooltip.h"
#include <boost/shared_ptr.hpp>


class Thread;
class NodeData;


/**
 * A class for displaying the threads of a debugged program.
 * Threads are shown by pid, hierarchically  organized in
 * a tree.
 * @note a more apt name for this class is RunnableView -- the
 * text on the label on top of it actually says "Runnables"; the
 * threads are shown by their task id the kernel knowns them by.
 */
class ZDK_LOCAL ThreadView : public Gtk::ScrolledWindow
{
    class ToolTipTraits;

public:
    ThreadView(const std::string&, Gtk::Window&);

    ~ThreadView();

    SigC::Signal2<void, pid_t, unsigned long> selection_changed;

    void clear();

    bool add_thread(const Thread&);

    bool remove(pid_t);

    bool select(pid_t);

    /**
     * update thread info (runs on main debugger thread)
     */
    void update(const Thread&);

    /**
     * apply all updates (runs on UI thread)
     */
    void display();

private:
    void on_selection_changed(Gtk::CTree::Row, int);

    struct Item
    {
        std::vector<std::string> col;
        void* userData;
    };
    Item make_tree_item(const Thread&);

    void popup_menu(GdkEventButton& rightClickEvent, Thread*);

    event_result_t on_button_press_event(GdkEventButton*);

    void on_tree_expand(Gtk::CTree::Row);

    void set_icon(Gtk::CTree::Row, bool frozen);

    void show_task_info(const Runnable*);

#ifdef GTKMM_2
    void on_toggle(const Glib::ustring& path);
#endif

private:
    /* when true, it means the selection has changed because
       items were added or removed programmatically, in contrast
       to the user interacting with the widget */
    bool programmaticSelect_;

    boost::shared_ptr<ToolTipped<Gtk::CTree, ToolTipTraits> > tree_;

    std::vector<boost::shared_ptr<NodeData> > data_;

    Gtk::Window* top_;

    // for updating thread info
    typedef std::queue<std::pair<pid_t, std::string> > UpdateQueue;

    UpdateQueue updateQueue_;
};

#endif // THREAD_VIEW_H__AF353FD4_BA41_4D44_8FA1_02A7133F5B94
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
