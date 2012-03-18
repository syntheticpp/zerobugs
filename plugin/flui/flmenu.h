#ifndef FLMENU_H__93D60C08_34D0_4D9C_98D1_19BBD50428EA
#define FLMENU_H__93D60C08_34D0_4D9C_98D1_19BBD50428EA
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "menu.h"
#include <FL/Fl_Menu_Bar.H>


////////////////////////////////////////////////////////////////
class FlCompositeMenu : public ui::CompositeMenu
{
protected:
    explicit FlCompositeMenu(ui::Controller&);

    ~FlCompositeMenu() throw();

    void exec_command(const char* path);

private:
    ui::Controller& controller_;
};



////////////////////////////////////////////////////////////////
class FlMenuBar : public FlCompositeMenu
{
public:
    FlMenuBar(ui::Controller&, int w, int h);

protected:
    ~FlMenuBar() throw();

    virtual void add(
            const std::string&  name,
            int                 shortcut,
            ui::EnableCondition enable,
            RefPtr<ui::Command> command);

    using FlCompositeMenu::exec_command;

    static void exec_command(Fl_Widget*, void*);

private:
    Fl_Menu_*       menu_;
};


////////////////////////////////////////////////////////////////
class FlPopupMenu : public FlCompositeMenu
{
public:
    explicit FlPopupMenu(ui::Controller&);

    // pop goes the weasel
    void show(int x, int y);

protected:
    ~FlPopupMenu() throw();

    virtual void add(
            const std::string&  name,
            int                 shortcut,
            ui::EnableCondition enable,
            RefPtr<ui::Command> command);

private:
    std::vector<Fl_Menu_Item>   items_;
};


////////////////////////////////////////////////////////////////

class FlMenuItem : public ui::MenuItem
{
public:
    FlMenuItem(
        const std::string&  name,
        int                 shortcut,
        ui::EnableCondition enable,
        RefPtr<ui::Command> command,
        Fl_Menu_*           menu,
        int                 index );

protected:
    ~FlMenuItem() throw() { }

    virtual void enable(bool);

private:
    Fl_Menu_*       menu_;
    int             index_;
};

#endif // FLMENU_H__93D60C08_34D0_4D9C_98D1_19BBD50428EA

