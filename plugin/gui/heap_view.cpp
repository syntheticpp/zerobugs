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
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <boost/format.hpp>
#include "gtkmm/box.h"
#include "gtkmm/clist.h"
#include "gtkmm/connect.h"
#include "gtkmm/ctree.h"
#include "gtkmm/flags.h"
#include "gtkmm/label.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "zdk/check_ptr.h"
#include "zdk/zero.h"
#include "zdk/heap.h"
#include "program_view.h"
#include "scope_helpers.h"
#include "slot_macros.h"
#include "heap_view.h"

using namespace std;
using namespace SigC;


////////////////////////////////////////////////////////////////
HeapView::HeapView(Debugger& debugger, ProgramView& view)
    : DialogBox(btn_ok, "Heap View")
    , debugger_(debugger)
    , view_(view)
    , heap_(0)
    , box_(manage(new Gtk::VBox))
    , label_(manage(new Gtk::Label("", .0)))
    , tree_(0)
{
    get_vbox()->add(*box_);
    box_->pack_start(*label_, false, false);

    sortType_[0] = sortType_[1] = GTK_SORT_ASCENDING;

    debugger.enum_plugins(this);

    if (!heap_)
    {
        CHKPTR(label_)->set_text("Heap plugin not active.");
    }
    box_->set_border_width(10);
    Gtk_set_resizable(this, true);
}


////////////////////////////////////////////////////////////////
void HeapView::notify(DebuggerPlugin* plugin)
{
    assert(plugin);
    if (Heap* heap = interface_cast<Heap*>(plugin))
    {
        heap_ = heap;
        init_tree();

        assert(tree_);
        ScopedFreeze<Gtk::CTree>__(*tree_);

        const size_t nblocks = heap->enum_blocks(this);

        ostringstream strm;
        strm << "Heap usage: "
             << heap->total() << " byte(s) in "
             << nblocks << " memory block(s)\n";

        CHKPTR(label_)->set_text(strm.str());
    }
}


////////////////////////////////////////////////////////////////
BEGIN_SLOT(HeapView::notify,(const HeapBlock* blk))
{
    assert(blk);

    vector<string> items;
    items.push_back((boost::format("0x%0x") % blk->addr()).str());
    items.push_back((boost::format("%1% byte")
        % blk->size()).str() + (blk->size() == 1 ? "" : "s"));

    Gtk::CTree::RowList rows(tree_->rows());
    rows.push_back(Gtk::CTree::Element(items));
    rows.back().set_data(const_cast<HeapBlock*>(blk));

    static const vector<string> dummy(2);
    rows = rows.back().subtree();
    rows.push_back(Gtk::CTree::Element(dummy));
}
END_SLOT()


////////////////////////////////////////////////////////////////
void HeapView::init_tree()
{
    Gtk::ScrolledWindow* sw = manage(new Gtk::ScrolledWindow());
    box_->pack_start(*sw, true, true);
    Gtk_set_size(this, 450, 250);

    static const char* labels[] = {
        "Address", "Size / Stack Trace", 0
    };

    tree_ = manage(new Gtk::CTree(labels));
    sw->add(*tree_);
    sw->set_policy(Gtk_FLAG(POLICY_NEVER), Gtk_FLAG(POLICY_AUTOMATIC));

    tree_->column(0).set_width(160);
    tree_->column(0).set_passive();

    Gtk_CONNECT_0(tree_, tree_expand, this, &HeapView::on_expand);
    Gtk_CONNECT_0(tree_, tree_select_row, this, &HeapView::on_select_row);

#if !GTKMM_2
    tree_->set_auto_sort(false);
    tree_->set_line_style(Gtk_FLAG(CTREE_LINES_NONE));
    tree_->set_selection_mode(Gtk_FLAG(SELECTION_SINGLE));
#endif
    Gtk_CONNECT_0(tree_, click_column, this, &HeapView::on_column_click);
}


////////////////////////////////////////////////////////////////
void HeapView::on_expand(Gtk::CTree::Row row)
{
    using namespace Gtk::CTree_Helpers;

    // populate the tree if needed
    if (row.get_data() && row.subtree().front()[0].get_text().empty())
    {
        HeapBlock* block = reinterpret_cast<HeapBlock*>(row.get_data());

        RowList subtree = row.subtree();
        subtree.erase(subtree.begin());

        if (RefPtr<StackTrace> trace = block->stack_trace())
        {
            const size_t n = trace->size();
            for (size_t i = 0; i != n; ++i)
            {
                vector<string> items;
                ostringstream outs;

                if (Frame* frame = trace->frame(i))
                {
                    ostringstream addr;
                    addr << "0x" << hex << setw(sizeof(addr_t) * 2);
                    addr << setfill('0') << frame->program_count();

                    items.push_back(addr.str());

                    if (RefPtr<Symbol> sym = frame->function())
                    {
                        ostringstream source;
                        source << sym->file();

                        if (size_t line = sym->line())
                        {
                            source << ':' << line;
                        }
                        items.push_back(source.str());
                    }
                    else
                    {
                        items.push_back("???");
                    }

                    subtree.push_back(Gtk::CTree::Element(items));
                    subtree.back().set_data(frame->function());
                }
                else
                {
                    clog << "oops, null frame[" << i << "]\n";
                }
            }
            row.expand();
        }
        else
        {
            clog << "oops, null trace\n";
        }
        row.set_data(0);
    }
}


////////////////////////////////////////////////////////////////
void HeapView::on_column_click(int col)
{
    assert(col< 2);
    assert(col>= 0);

    if (heap_ && col == 1)
    {
        ScopedFreeze<Gtk::CTree>__(*tree_);

        tree_->clear();

        Heap::EnumOption opt = sortType_[col]
            ? Heap::SORT_SIZE_DECREASING
            : Heap::SORT_SIZE_INCREASING;

        heap_->enum_blocks(this, opt);
    }
    sortType_[col] ^= 1; // toggle
}


////////////////////////////////////////////////////////////////
void HeapView::on_select_row(Gtk::CTree::Row row, int)
{
    string text = row[1].get_text();

    if (!isdigit(text[0])) // is block size?
    {
        // no, must be a stack trace entry
        // assert(row.get_data());
        if (row.get_data())
        {
            Symbol* sym = reinterpret_cast<Symbol*>(row.get_data());

            // todo: should the heap blocks also record the thread?
            RefPtr<Thread> thread(debugger_.get_thread(DEFAULT_THREAD));
            view_.show_function(sym, thread);
        }
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
