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
#include <assert.h>
#include <sstream>
#include "gtkmm/connect.h"
#include "gtkmm/ctree.h"
#include "gtkmm/flags.h"
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "zdk/variant_util.h"
#include "zdk/zero.h"
#include "generic/lock.h"
#include "edit_in_place.h"
#include "highlight_changes.h"
#include "scope_helpers.h"
#include "slot_macros.h"
#include "register_view.h"

using namespace std;

// Column Titles
static const char* titles[] = { "Register", "Value", 0 };


////////////////////////////////////////////////////////////////
RegisterView::RegisterView()
    : list_(new EditableList(titles))
    , expandFlags_(true)
    , hiliteColor_(highlight_changes_color())
{
    set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_AUTOMATIC));
    list_->set_column_editable(1, true);

    this->add(*list_);

    // for memorizing the state of flag register:
    // expanded or collapsed
    Gtk_CONNECT_0(list_, tree_collapse, this, &RegisterView::on_collapse);
    Gtk_CONNECT_0(list_, tree_expand, this, &RegisterView::on_expand);

    list_->column(0).set_width(100);
    list_->column(0).set_passive();
    list_->column(1).set_passive();

    Gtk_CONNECT_0(list_, cell_edit, this, &RegisterView::on_cell_edit);
#if GTKMM_2
    list_->set_grid_lines(Gtk::TREE_VIEW_GRID_LINES_BOTH);
#endif
}


RegisterView::~RegisterView() throw()
{
}


/**
 * Fetch the current values stored in the CPU regs
 */
void RegisterView::update(RefPtr<Thread> thread)
{
    Lock<Mutex> lock(mutex_);
    regs_.clear();

    if (thread && !thread_finished(*thread))
    {
        if (thread_ != thread)
        {
            thread_ = thread;
            oldValues_.clear();
        }
        thread->enum_cpu_regs(this);
    }
}


void RegisterView::notify(Register* reg)
{
    Lock<Mutex> lock(mutex_);

    regs_.push_back(CHKPTR(reg));
    assert(list_.get());
}


void RegisterView::clear()
{
    Lock<Mutex> lock(mutex_);

    thread_.reset();
    regs_.clear();
}


namespace
{
    /**
     * For displaying flag registers
     */
    class ZDK_LOCAL FlagsHelper
        : public EnumCallback3<const char*, reg_t, reg_t>
    {
        Gtk::CTree::Row&    row_;
        const Gdk_Color     hiliteCol_;
        Register*           reg_;
        reg_t               oldVal_;

    public:
        template<typename T>
        FlagsHelper(Gtk::CTree::Row& row, Gdk_Color col, Register* r, T oldVal)
            : row_(row)
            , hiliteCol_(col)
            , reg_(r)
            , oldVal_(static_cast<reg_t>(oldVal))
        { }

        void notify(const char* name, reg_t flag, reg_t mask)
        {
            vector<string> f;

            f.push_back(name);
            ostringstream v;
            v << hex << flag;
            f.push_back(v.str());

            row_.subtree().push_back(Gtk::CTree::Element(f));
            Gtk::CTree::Row row = row_.subtree().back();
            if (flag != (oldVal_ & mask))
            {
                row.set_foreground(hiliteCol_);
            }
            row.set_data(reg_);
        }
    };
}


/**
 * Synchronize the contents of the widget with the registers
 */
void RegisterView::display()
{
    Lock<Mutex> lock(mutex_);
    ScopedFreeze<Gtk::CTree> scopedFreeze(*list_);
    list_->clear();

    bool firstTime = false;

    if (oldValues_.empty())
    {
        oldValues_.resize(regs_.size());
        firstTime = true;
    }

    vector<RefPtr<Register> >::const_iterator i = regs_.begin();
    vector<reg_value_type>::iterator j = oldValues_.begin();

    for (; i != regs_.end(); ++i, ++j)
    {
        vector<string> tmp;
        tmp.push_back((*i)->name());

        ostringstream out;
        reg_value_type value = 0;

        RefPtr<Variant> v = (*i)->value();

        // todo: make the numeric base user-configurable
        variant_print(out, *v, 16);

        value = is_float(*v) ? v->long_double() : v->uint64();

        tmp.push_back(out.str());
        list_->rows().push_back(Gtk::CTree::Element(tmp));

        assert(j != oldValues_.end());

        Gtk::CTree::Row row = list_->rows().back();
        row.set_data(static_cast<Register*>(i->get()));
        if (!firstTime && (value != *j))
        {
            // use a different color to hilight the
            // registers that have changed
            row.set_foreground(hiliteColor_);
        }

        FlagsHelper helper(row, hiliteColor_, i->get(), (firstTime ? value : *j));
        if ((*i)->enum_fields(&helper))
        {
            if (expandFlags_)
            {
                row.expand();
            }
        }
        *j = value;
    }
}


/**
 * Commit the new value to the register (a pointer to the
 * Register interface has been stored as the row's data).
 * @note this widget doesn't call Register::set_value directly,
 * it rather emits a signal; the slot associated to the
 * signal is responsible to marshal the set_value call to
 * the main thread. We cannot call any method that may
 * result in ptrace calls from the UI thread, they need to
 * be invoked from the main debugger thread.
 */
BEGIN_SLOT_(bool, RegisterView::on_cell_edit,( CELL_EDIT_PARAM ))
{
    Lock<Mutex> lock(mutex_);

    Gtk::CTree::Row row = list_->row(path);
    edit_cell(row, s);
}
END_SLOT_(false)


void
RegisterView::edit_cell(Gtk::CTree::Row& row, const string& val)
{
    Lock<Mutex> lock(mutex_);

    if (void* data = row.get_data())
    {
        Register* reg = reinterpret_cast<Register*>(data);
        set_value(reg, val, row[0].get_text());
    }
}


void RegisterView::on_collapse(Gtk::CTree::Row)
{
    //Lock<Mutex> lock(mutex_);
    expandFlags_ = false;
}


void RegisterView::on_expand(Gtk::CTree::Row)
{
    //Lock<Mutex> lock(mutex_);
    expandFlags_ = true;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
