//
// $Id: thread_view.cpp 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdio.h>      // snprintf
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include "zdk/zero.h"   // for Thread interface definition
#include "generic/temporary.h"
#include "popup_menu.h"
// gtkmm compatibility headers
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/frame.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "slot_macros.h"
#include "text_dialog.h"
#include "thread_view.h"

using namespace std;
using namespace Gtk;
using namespace SigC;


static const char* colTitle[] = { "Thread", "Name", 0 };

struct ZDK_LOCAL NodeData
{
    pid_t       pid_;
    pid_t       ppid_;
    pthread_t   ptid_;
    string      name_;
    string      filename_;
    Debugger*   dbg_;

    explicit NodeData(const Thread& thread)
      : pid_(thread.lwpid())
      , ppid_(thread.ppid())
      , ptid_(thread.thread_id())
      , dbg_(thread.debugger())
    {
        if (const char* name = thread.name())
        {
            name_ = name;
        }
        if (const char* name = thread.filename())
        {
            filename_ = name;
        }
    }
};


/* A thread has a pthread id, by which it is known to the pthreads
   library and the user code, and a pid (process id or task id)
   by which it is known to the kernel. */

class ZDK_LOCAL ThreadView::ToolTipTraits
{
public:
    static bool get_text_at_pointer
    (
        Gtk::Widget&    wid,
        double          x,
        double          y,
        Gtk::RowHandle& hrow,
        int&            hcol,
        string&         text
    )
    {
        bool result = false;
        text = "";
        Gtk::CTree& tw = dynamic_cast<Gtk::CTree&>(wid);
        Gtk::RowHandle nrow;
        int ncol = 0;

        if (tw.get_selection_info((int)x, (int)y, &nrow, &ncol))
        {
            if (hcol != ncol || !ROW_HANDLE_EQUAL(hrow, nrow))
            {
                hrow = nrow, hcol = ncol;

                Gtk::CTree::Row row = tw.rows()[nrow];

                if (NodeData* nodeData = reinterpret_cast<NodeData*>(row.get_data()))
                {
                    ostringstream os;

                    if (nodeData->ptid_ == 0)
                    {
                        const pid_t lwpid = nodeData->ppid_;
                        const pthread_t tid = nodeData->ptid_;

                        if (Thread* thread = nodeData->dbg_->get_thread(lwpid, tid))
                        {
                            nodeData->ptid_ = thread->thread_id();
                        }
                    }
                    if (nodeData->ptid_)
                    {
                        os << "thread ID: " << nodeData->ptid_ << " ";
                    }
                    os << nodeData->filename_;

                    text = os.str();
                    result = true;
                }
            }
        }
        return result;
    }
};



////////////////////////////////////////////////////////////////
ThreadView::ThreadView(const string& label, Gtk::Window& top)
    : programmaticSelect_(false)
    , tree_(new ToolTipped<Gtk::CTree, ToolTipTraits>(colTitle))
    , top_(&top)
{
    set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_AUTOMATIC));
    add(*tree_);

    tree_->column(0).set_passive();

#ifdef GTKMM_2
    tree_->column(0).set_width(120);
    tree_->column(1).set_width(64);

    // the CTree adapter class has some "hidden" data columns in
    // the model; here we overload the "selectable" column: if the
    // flag is not set then we ignore debug events for the corresponding
    // thread
    CellRendererToggle* renderer = manage(new CellRendererToggle);
    tree_->append_column("Debug", *renderer);
    tree_->get_column(2)->add_attribute(renderer->property_active(),
                                        tree_->selectable());
    renderer->signal_toggled().connect(mem_fun(*this, &ThreadView::on_toggle));
    tree_->column(2).set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
#endif

    tree_->set_selection_mode(Gtk_FLAG(SELECTION_SINGLE));
    Gtk_CONNECT_0(  tree_,
                    tree_select_row,
                    this,
                    &ThreadView::on_selection_changed);
}


////////////////////////////////////////////////////////////////
ThreadView::~ThreadView()
{
    clear();
}


////////////////////////////////////////////////////////////////
void ThreadView::on_selection_changed(Gtk::CTree::Row row, int)
{
    if (programmaticSelect_)
    {
        return;
    }
    if (const NodeData* data = reinterpret_cast<NodeData*>(row.get_data()))
    {
        selection_changed(data->pid_, data->ptid_);
    }
}


////////////////////////////////////////////////////////////////
void ThreadView::clear()
{
    Temporary<bool> tmp(programmaticSelect_, true);
    Gtk::clear_rows(*tree_);
    data_.clear();
}


////////////////////////////////////////////////////////////////
ThreadView::Item ThreadView::make_tree_item(const Thread& thread)
{
    ostringstream os;
    os << thread.lwpid();

    Thread* mainThread = NULL;
    if (RefPtr<Process> proc = thread.process())
    {
        mainThread = proc->get_thread(DEFAULT_THREAD);
    }

    if (thread.is_execed())
    {
        os << " (exec)";
    }
    else if (thread.is_forked())
    {
        os << " (fork)";
    }
    else if (!mainThread || (&thread == mainThread))
    {
        // main thread - do nothing
    }
    else
    {
        os << " (thread)";
    }
    Item item;

    item.col.push_back(os.str());
    item.col.push_back(thread.name());

    boost::shared_ptr<NodeData> nodeData(new NodeData(thread));
    data_.push_back(nodeData);
    item.userData = nodeData.get();

    if (pthread_t ptid = thread.thread_id())
    {
        nodeData->ptid_ = ptid;
    }
    assert(nodeData->dbg_ == thread.debugger());
    return item;
}


////////////////////////////////////////////////////////////////
static bool
find(Gtk::CTree::RowList rows, pid_t pid, Gtk::CTree::Row& row)
{
    Gtk::CTree::RowList::iterator i = rows.begin();
    for (; i != rows.end(); ++i)
    {
#ifdef GTKMM_2
        if (!*i)
        {
            break;
        }
#endif
        row = Gtk::get_row(rows, *i);
        if (NodeData* data = reinterpret_cast<NodeData*>(row.get_data()))
        {
            if (data->pid_ == pid)
            {
                return true;
            }
        }
        if (find(row.subtree(), pid, row))
        {
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
bool find_child(Gtk::CTree::RowList rows, pid_t pid)
{
    Gtk::CTree::RowList::iterator i = rows.begin();
    for (; i != rows.end(); ++i)
    {
#ifdef GTKMM_2
        if (!*i)
        {
            break;
        }
#endif
        Gtk::CTree::Row row = Gtk::get_row(rows, *i);
        if (NodeData* data = reinterpret_cast<NodeData*>(row.get_data()))
        {
            if (data->ppid_ == pid)
            {
                return true;
            }
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
static bool delete_thread(Gtk::CTree::RowList rows, pid_t pid)
{
    Gtk::CTree::RowList::iterator i = rows.begin();
    while (i != rows.end())
    {
        Gtk::CTree::Row row = Gtk::get_row(rows, *i);
        const NodeData* data = reinterpret_cast<NodeData*>(row.get_data());

        //
        //todo: figure out how to re-parent CTree items
        //
        if (data && (data->pid_ == pid))
        {
            i = rows.erase(i);
            return true;
        }
        else
        {
            Gtk::CTree::RowList subtree = row.subtree();
            if (delete_thread(subtree, pid))
            {
                return true;
            }
        }
        ++i;
    }
    return false;
}


////////////////////////////////////////////////////////////////
bool ThreadView::add_thread(const Thread& thread)
{
    assert(tree_);
    CTree::RowList rows = tree_->rows();
 /*
    if (find_child(rows, thread.lwpid()))
    {
        return false;
    }
  */
    Gtk::CTree::Row row;
    if (thread.is_execed())
    {
        // the thread is replacing a fork-ed thread
        delete_thread(rows, thread.lwpid());
        assert (!find(rows, thread.lwpid(), row));
    }
    else
    {
        if (find(rows, thread.lwpid(), row))
        {
            return false;
        }
    }
    //
    // lookup the parent, by process id, in the tree
    //
    if (find(rows, thread.ppid(), row))
    {
        Item item = make_tree_item(thread);

        CTree::RowList subtree = row.subtree();
        subtree.push_back(CTree::Element(item.col));
        subtree.back().set_data(item.userData);
        set_icon(subtree.back(), false);

        row.expand();
    }
    else
    {
        Item elem = make_tree_item(thread);
        rows.push_back(CTree::Element(elem.col));

        row = rows.back();

        row.set_data(elem.userData);
        set_icon(row, false);
    }
    return true;
}


////////////////////////////////////////////////////////////////
// NOTE: runs on main thread
void ThreadView::update(const Thread& thread)
{
    updateQueue_.push(make_pair(thread.lwpid(), thread.name()));
}


////////////////////////////////////////////////////////////////
void ThreadView::display()
{
    assert(tree_);
    while (!updateQueue_.empty())
    {
        pair<pid_t, string> tinfo = updateQueue_.front();
        updateQueue_.pop();
        Gtk::CTree::Row row;
        if (find(tree_->rows(), tinfo.first, row))
        {
            row[1].set_text(tinfo.second);
        }
    }
}


////////////////////////////////////////////////////////////////
bool ThreadView::remove(pid_t pid)
{
    Temporary<bool> tmp(programmaticSelect_, true);
    return delete_thread(tree_->rows(), pid);
}


////////////////////////////////////////////////////////////////
bool ThreadView::select(pid_t pid)
{
    Gtk::CTree::Row row;
    bool result = find(tree_->rows(), pid, row);
    if (result)
    {
        if (tree_->selection().empty()
         || row != Gtk::get_selected_row(tree_->selection()))
        {
            Temporary<bool> tmp(programmaticSelect_, true);
            row.select();
        }
    }
    return result;
}


/*
static void toggle_debug(Thread* thread)
{
    if (thread)
    {
        bool flag = thread->is_traceable();
        flag ^= true;
        thread->set_traceable(flag);
    }
} */


////////////////////////////////////////////////////////////////
void ThreadView::popup_menu(GdkEventButton& event, Thread* thread)
{
    assert(thread);
    std::auto_ptr<PopupMenu> menu(new PopupMenu);

    {
        Gtk::MenuItem* item =
            menu->add_manage_item(new Gtk::MenuItem("Properties"));
        Gtk_CONNECT_1(item,
                      activate,
                      this,
                      &ThreadView::show_task_info,
                      get_runnable(thread, nothrow));
    }
  /*
    {
        Gtk::CheckMenuItem* item =
            menu->add_manage_item(new Gtk::CheckMenuItem("Debug"));
        if (thread->is_traceable())
        {
            item->set_active(true);
        }
        Gtk_CONNECT(item, toggled, Gtk_BIND(Gtk_PTR_FUN(toggle_debug), thread));
    } */
    menu.release()->popup(event.button, event.time);
}


////////////////////////////////////////////////////////////////
void ThreadView::set_icon(Gtk::CTree::Row row, bool frozen)
{
    // todo
}


////////////////////////////////////////////////////////////////
event_result_t
ThreadView::on_button_press_event(GdkEventButton* event)
{
    if ((event != NULL)                   &&
        (event->type == GDK_BUTTON_PRESS) &&
        (event->button == 3))
    {
        Gtk::RowHandle row;
        int col = 0; // not used

        // silence off compiler warnings; event->x and event->y
        // are of the double type, and get_selection_info expects
        // integers
        const int x = static_cast<int>(event->x);
        const int y = static_cast<int>(event->y);

        if (tree_ && tree_->get_selection_info(x, y, &row, &col))
        {
        #ifdef DEBUG
            clog << __func__ << "=(" << row << ", " << col << ")\n";
        #endif
            NodeData* data =
                reinterpret_cast<NodeData*>(tree_->row(row).get_data());

            if (data && data->dbg_)
            {
                const pid_t pid = data->pid_;
                const pthread_t tid = data->ptid_;

                if (Thread* thread = data->dbg_->get_thread(pid, tid))
                {
                    popup_menu(*event, thread);
                }
            }
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////
BEGIN_SLOT(ThreadView::show_task_info,(const Runnable* task))
{
    struct Helper : public EnumCallback2<int, const char*>
    {
        virtual ~Helper() { }
        ostringstream str_;

        void notify(int fd, const char* filename)
        {
            str_ << setw(5) << fd << "\t" << filename << endl;
        }
    };

    if (!task) return;
    Helper helper;
    helper.str_ << "pid:    " << task->pid() << "\n";
    helper.str_ << "group:  " << task->gid() << "\n";
    helper.str_ << "parent: " << task->ppid() << "\n";
    helper.str_ << "\nVirtual Memory: " << task->vmem_size() << "\n";
    helper.str_ << "\nOpen Files:\n";

    CHKPTR(task)->enum_open_files(&helper);

    TextDialog dlg("Thread Properties", 250, 180);
    dlg.insert_text(helper.str_.str());
    dlg.set_transient_for(*top_);
    dlg.run();
}
END_SLOT()


#ifdef GTKMM_2

////////////////////////////////////////////////////////////////
void ThreadView::on_toggle(const Glib::ustring& path)
{
    if (Gtk::TreeIter iter =
            CHKPTR(tree_->model())->get_iter(Gtk::TreeModel::Path(path)))
    {
        Gtk::TreeModel::Row row = *iter;

        bool flag = row[tree_->selectable()];
        flag ^= true;
        row[tree_->selectable()] = flag;

        boost::shared_ptr<void> data = row[tree_->user_data()];
        if (data)
        {
            NodeData* nd = reinterpret_cast<NodeData*>(data.get());
            Thread* thread = CHKPTR(nd->dbg_)->get_thread(nd->pid_, nd->ptid_);
            if (thread)
            {
                thread->set_traceable(flag);
            }
        }
    }
}
#endif
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
