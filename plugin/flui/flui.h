#ifndef Flui_H__70E93067_9421_4550_8C81_B35381DC19F6
#define Flui_H__70E93067_9421_4550_8C81_B35381DC19F6
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/priority.h"
#include "zdk/version_info_impl.h"
#include "controller.h"
#include "dharma/sarray.h"
#include "menu.h"

class FlMainWindow;


/**
 * Fltk-based user interface plugin.
 *
 * Implements ui::Controller.
 */
class Flui
    : public ui::Controller
    , public Priority
    , VersionInfoImpl<ZERO_API_MAJOR, ZERO_API_MINOR>
{
public:
    Flui();

protected:
    virtual ~Flui();

    BEGIN_INTERFACE_MAP(Flui)
        INTERFACE_ENTRY(DebuggerPlugin)
        INTERFACE_ENTRY(Priority)
        INTERFACE_ENTRY(VersionInfo)
    END_INTERFACE_MAP()

    int x() const;
    int y() const;
    int w() const;
    int h() const;

    using ui::Controller::show_edit_breakpoint_dialog;

    void show_edit_breakpoint_dialog(ui::UserBreakPoint&);
    void show_eval_dialog();

    virtual void error_message(const std::string&);

    // --- RefCounted
    virtual void release();

    virtual void run();

    // -- Priority interface
    Priority::Class priority_class() const {
        return Priority::EXPERIMENTAL;
    }

    // --- DebuggerPlugin interface
    /**
     * Pass pointer to debugger and the command line params
     * to plug-in module.
     */
    virtual bool initialize(Debugger*, int* argc, char*** argv);

    // -- VersionInfo interface
    const char* description() const;

    const char* copyright() const;

private:
    virtual RefPtr<ui::BreakPointView>  init_breakpoint_view();
    virtual RefPtr<ui::CodeView>        init_code_view();
    virtual RefPtr<ui::PopupMenu>       init_contextual_menu();
    virtual RefPtr<ui::Layout>          init_layout();
    virtual RefPtr<ui::VarView>         init_locals_view();
    virtual RefPtr<ui::CompositeMenu>   init_menu();
    virtual RefPtr<ui::StackView>       init_stack_view();
    virtual RefPtr<ui::Toolbar>         init_toolbar();

    virtual void init_main_window(int x, int y, int w, int h);

    virtual void lock();
    virtual void unlock();
    virtual void notify_ui_thread();

    virtual int  wait_for_event();

private:
    SArray          args_;
    FlMainWindow*   window_;
};


#endif // Flui_H__70E93067_9421_4550_8C81_B35381DC19F6
