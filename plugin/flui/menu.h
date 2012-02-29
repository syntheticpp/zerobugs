#ifndef MENU_H__131240E4_5274_4C57_8206_9572CF1CE829
#define MENU_H__131240E4_5274_4C57_8206_9572CF1CE829
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "command.h"
#include "view.h"
#include <functional>
#include <string>
#include <vector>


namespace ui
{
    /**
     * Toolkit-agnostic menu element.
     */
    class MenuElem : public RefCountedImpl<>
    {
    protected:
        MenuElem()
        { }

        explicit MenuElem(const std::string& name, int shortcut)
            : name_(name)
            , shortcut_(shortcut)
        { }

        ~MenuElem() throw()
        { }

    public:
        enum EnableCondition
        {
            Enable_Always,
            Enable_IfStopped,
            Enable_IfRunning,
        };

        const std::string& name() const
        {
            return name_;
        }

        int shortcut() const
        {
            return shortcut_;
        }

        virtual void update(const State& state)
        {
        }

        virtual RefPtr<Command> emit_command() const;

    private:
        std::string name_;
        int         shortcut_;  // aka accelerator
    };



    class MenuItem : public MenuElem
    {
    public:
        MenuItem(
            const std::string&  name,
            int                 shortcut,
            EnableCondition     enable,
            RefPtr<Command>     command );

    protected:
        ~MenuItem() throw()
        { }

        virtual RefPtr<Command> emit_command() const
        {
            command_->reset();
            return command_;
        }

        virtual void update(const State&);

        virtual void enable(bool) { }

    private:
        RefPtr<Command> command_;
        EnableCondition enableCond_;
    };



    /**
     * Base class for menu-bar, contextual menus, etc.
     * Aggregates sub-menus or menu elements.
     */
    class CompositeMenu : public MenuElem
    {
    public:
        CompositeMenu()
        { }

        explicit CompositeMenu(const std::string& name, int shortcut = 0)
            : MenuElem(name, shortcut)
        { }

    protected:
        ~CompositeMenu() throw()
        { }

        void add(RefPtr<MenuElem> menu);

    public:
        template<typename T> void add_item(
            const std::string&  name,
            int                 shortcut,
            EnableCondition     enable,
            T                   callable )
        {
            add(name, shortcut, enable, new MainThreadCommand<T>(callable));
        }

        template<typename T> void add_ui_item(
            const std::string&  name,
            int                 shortcut,
            EnableCondition     enable,
            T                   callable )
        {
            add(name, shortcut, enable, new UIThreadCommand<T>(callable));
        }

        virtual void update(const State& state);

    protected:
        virtual void add(
            const std::string&  name,
            int                 shortcut,
            EnableCondition     enable,
            RefPtr<Command>     command ) = 0;

        std::vector<RefPtr<MenuElem> >  children_;
    };
}

#endif // MENU_H__131240E4_5274_4C57_8206_9572CF1CE829
