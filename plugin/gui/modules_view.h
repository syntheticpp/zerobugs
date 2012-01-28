#ifndef MODULES_VIEW_H__3C0D1DA7_8D24_4637_81EB_0E1E557AEB3D
#define MODULES_VIEW_H__3C0D1DA7_8D24_4637_81EB_0E1E557AEB3D
//
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
#include "zdk/weak_ptr.h"
#include "dialog_box.h"

class Debugger;
class Module;
class FileSel; // file selection, for adding shared objects

namespace Gtk
{
    class CList;
    class ScrolledWindow;
}

/**
 * Displays the modules of the debugged program, and the
 * memory addresses where they are loaded. A module may
 * be the main executable, a shared object (aka dynamic library),
 * or any other file that the debugged program has loaded with
 * the dlopen() system call.
 */
class ModulesView : public DialogBox
{
public:
    ModulesView();

    void populate_using(Process&);

    void clear();

    void add_module(const Module&);

private:
    void add_shared_object();

    void on_add_shared_object(FileSel*);

private:
    Gtk::CList* list_;
    Gtk::ScrolledWindow* sw_;

    WeakPtr<Process> proc_;
};

#endif // MODULES_VIEW_H__3C0D1DA7_8D24_4637_81EB_0E1E557AEB3D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
