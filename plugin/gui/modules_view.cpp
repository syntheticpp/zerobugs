//
// $Id$
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <set>
#include <sstream>
#include "dharma/canonical_path.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/clist.h"
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/resize.h"
#include "gtkmm/sorter.h"
#include "gtkmm/style.h"
#include "gtkmm/widget.h" // Gtk_POPDOWN
#include "zdk/check_ptr.h"
#include "zdk/zero.h"
#include "file_selection.h"
#include "fixed_font.h"
#include "modules_view.h"
#include "slot_macros.h"

using namespace std;




ModulesView::ModulesView()
    : DialogBox(btn_close, "Modules")
    , list_(0)
    , sw_(manage(new Gtk::ScrolledWindow))
{
    sw_->set_policy(Gtk_FLAG(POLICY_AUTOMATIC),
                   Gtk_FLAG(POLICY_AUTOMATIC));

    get_vbox()->add(*sw_);

    static const char* columnTitles[] =
    {
        "Load address",
        "End address",
        "Adjustment",
        "File name",
        NULL
    };

    list_ = manage(new Gtk::CList(columnTitles));
    sw_->add(*list_);

    list_->column(0).set_width(115);
    list_->column(1).set_width(115);
    list_->column(2).set_width(100);

    set_numeric_sort(*list_, 0, 16);
    set_numeric_sort(*list_, 1, 16);
    set_numeric_sort(*list_, 2, 16);

    Gtk_set_size(sw_, 550, 200);
    Gtk_set_resizable(this, true);

#ifdef GTKMM_2
    Glib::RefPtr<Gtk::Style> style = CHKPTR(list_->get_style())->copy();
    list_->set_style(style);
#else
    list_->column(3).set_passive();
    Gtk::Style* style = CHKPTR(list_->get_style())->copy();
    style->set_font(Gdk_Font(fixed_font()));
    list_->set_style(*style);
#endif
    Gtk::Button* btn = add_button("_Add Shared Object");
    Gtk_CONNECT_0(btn, clicked, this, &ModulesView::add_shared_object);
    get_button_box()->set_layout(Gtk_FLAG(BUTTONBOX_END));
}


void ModulesView::clear()
{
    CHKPTR(list_)->clear();
}


void ModulesView::add_module(const Module& module)
{
    vector<string> cols;
    {
        ostringstream addr;
        if (module.addr())
        {
            addr << hex << module.addr();
        }
        else
        {
            // zero means the module is not loaded
            // (mapped into memory) yet
            addr << "n/a";
        }
        cols.push_back(addr.str());
    }
    {
        ostringstream addr;
        addr << hex << module.upper();
        cols.push_back(addr.str());
    }
    {
        ostringstream addr;
        addr << hex << module.adjustment();
        cols.push_back(addr.str());
    }
    cols.push_back(module.name()->c_str());
    list_->rows().push_back(cols);
}


class ModuleCallback : public EnumCallback<Module*>
{
public:
    explicit ModuleCallback(ModulesView* view) : view_(view)
    {
    }
    void notify(Module* module)
    {
        if (module)
        {
            string name = canonical_path(CHKPTR(module->name())->c_str());
            if (names_.insert(name).second)
            {
                CHKPTR(view_)->add_module(*module);
            }
        }
    }

private:
    ModulesView* view_;
    set<string>  names_;
};


void ModulesView::populate_using(Process& process)
{
    clear();
    ModuleCallback callback(this);
    process.enum_modules(&callback);

    proc_ = &process;
}


BEGIN_SLOT(ModulesView::add_shared_object,())
{
    FileSel fsel("Add Shared Object", get_icon());
    fsel.set_transient_for(*this);

    close_on_cancel(fsel);

    Gtk_CONNECT_SLOT(&fsel, destroy, Gtk::Main::quit);
    Gtk_CONNECT_1(fsel.get_ok_button(), clicked,
                  this, &ModulesView::on_add_shared_object, &fsel);

    fsel.hide_fileop_buttons();
    fsel.complete("*.so*");
    fsel.show();
    fsel.set_modal(true);

    Gtk::Main::run();
}
END_SLOT()


BEGIN_SLOT(ModulesView::on_add_shared_object,(FileSel* fsel))
{
    if (fsel)
    {
        string module = fsel->get_filename();
        Gtk_POPDOWN(fsel);

        if (RefPtr<Process> proc = proc_.ref_ptr())
        {
            if (SymbolMap* symbols = proc->symbols())
            {
                symbols->add_module(module.c_str());
                populate_using(*proc);
            }
        }
    }
}
END_SLOT()

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
