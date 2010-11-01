#ifndef REGISTER_VIEW_H__2431B92F_0D0F_4F3A_B3D7_78A11C6B1422
#define REGISTER_VIEW_H__2431B92F_0D0F_4F3A_B3D7_78A11C6B1422
//
// $Id: register_view.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "gtkmm/color.h"
#include "gtkmm/scrolledwindow.h"
#include "zdk/enum.h"
#include "zdk/export.h"
#include "zdk/mutex.h"


class Thread;

namespace Gtk
{
    class CTree;
}

template<typename T> class EditableInPlace;


/**
 * Displays the current values of the CPU registers.
 */
class ZDK_LOCAL RegisterView
    : EnumCallback<Register*>
    , public Gtk::ScrolledWindow
{
    typedef EditableInPlace<Gtk::CTree> EditableList;

public:
    RegisterView();

    virtual ~RegisterView() throw();

    void update(RefPtr<Thread> thread);

    void clear();

    void display();

    SigC::Signal3<void,
                    RefPtr<Register>,
                    std::string,    // value
                    std::string     // field name
                 > set_value;

    // SigC::Signal1<void, addr_t> show_memory;

protected:
    void display_flags(reg_t flags, reg_t oldFlags);

    bool on_cell_edit( CELL_EDIT_PARAM );

    void notify(Register*);

    void on_collapse(Gtk::CTree::Row);

    void on_expand(Gtk::CTree::Row);

private:
    void edit_cell(Gtk::CTree::Row& row, const std::string&);

    boost::shared_ptr<EditableList> list_;

    std::vector<RefPtr<Register> > regs_;

    // memorize the register values, so that we can compare
    // and show the changes in a highlighted color; use the
    // largest type
    // todo: use a variant instead?
    typedef long double reg_value_type;

    std::vector<reg_value_type> oldValues_;

    RefPtr<Thread> thread_;

    bool expandFlags_;

    const Gdk_Color hiliteColor_; // for showing changes

    // todo: numeric base
    Mutex mutex_;
};

#endif // REGISTER_VIEW_H__2431B92F_0D0F_4F3A_B3D7_78A11C6B1422
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
