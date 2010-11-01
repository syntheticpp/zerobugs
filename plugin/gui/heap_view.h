#ifndef HEAP_VIEW_H__482CAD1D_BD3A_4E42_B082_7FE33ACB7FEF
#define HEAP_VIEW_H__482CAD1D_BD3A_4E42_B082_7FE33ACB7FEF
//
// $Id: heap_view.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <vector>
#include "gtkmm/ctree.h"

#include "zdk/ref_ptr.h"
#include "dialog_box.h"

// zdk/zero.h
class Debugger;
class Symbol;

// zdk/heap.h
class Heap;
class HeapBlock;

class ProgramView;

// gtkmm
namespace Gtk
{
    class Box;
    class CTree;
    class Label;

    namespace CTree_Helpers
    {
        class Row;
    }
}

/**
 * Integrates the memory plugin in the UI;
 * this dialog displays the heap blocks that
 * are currently in use.
 */
class HeapView
    : public DialogBox
    , EnumCallback<DebuggerPlugin*>
    , EnumCallback<const HeapBlock*>
{
public:
    HeapView(Debugger&, ProgramView&);

protected:
    void notify(DebuggerPlugin*);

    void notify(const HeapBlock*);

    void init_tree();

    void on_expand(Gtk::CTree::Row);

    void on_column_click(int);

    void on_select_row(Gtk::CTree::Row, int);

private:
    Debugger&       debugger_;
    ProgramView&    view_;
    Heap*           heap_;
    Gtk::Box*       box_;
    Gtk::Label*     label_;
    Gtk::CTree*     tree_;
    int             sortType_[2];

    // for managing Symbol object's life-time
    std::vector<RefPtr<Symbol> > syms_;
};

#endif // HEAP_VIEW_H__482CAD1D_BD3A_4E42_B082_7FE33ACB7FEF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
