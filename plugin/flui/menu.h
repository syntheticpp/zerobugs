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
    class Menu : public RefCountedImpl<>
    {
    public:
        Menu()
        {
        }

        explicit Menu(const std::string& name, int shortcut) 
            : name_(name)
            , shortcut_(shortcut)
        {
        }

        ~Menu() throw()
        {
        }

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


    /**
     * Execute a command on the main thread, no continuation
     * on the UI thread.
     */
    template<typename Callable = std::function<void ()> >
    class SimpleCommandMenu : public Menu
    {
    public:
        SimpleCommandMenu(const std::string& name, int shortcut, Callable c)
            : Menu(name, shortcut)
            , callable_(c)
        {
        }

        ~SimpleCommandMenu() throw()
        {
        }
        
        virtual RefPtr<Command> emit_command() const
        {
            return new MainThreadCommand<Callable>(callable_);
        }

    private:
        Callable callable_;
    };

   
    template<typename T> inline
    RefPtr<SimpleCommandMenu<T> > simple_command_menu(
    
        const std::string&  name,
        int                 shortcut,
        T                   callable )
    {
        return new SimpleCommandMenu<T>(name, shortcut, callable);
    }


    class CompositeMenu : public Menu
    {
    public:
        CompositeMenu()
        {
        }

        CompositeMenu(const std::string& name, int shortcut = 0) 
            : Menu(name, shortcut)
        {
        }

        ~CompositeMenu() throw()
        {
        }
        
        virtual void add(RefPtr<Menu> menu) 
        {
            children_.push_back(menu);
        }
        
        virtual void update(const State& state) 
        {
            for (auto i = children_.begin(); i != children_.end(); ++i)
            {
                (*i)->update(state);
            }
        }
        //todo: add protected lookup method
    protected:
        std::vector<RefPtr<Menu> >  children_;
    };
}

#endif // MENU_H__131240E4_5274_4C57_8206_9572CF1CE829
