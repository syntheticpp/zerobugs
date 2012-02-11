#ifndef MENU_H__131240E4_5274_4C57_8206_9572CF1CE829
#define MENU_H__131240E4_5274_4C57_8206_9572CF1CE829
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/ref_counted_impl.h"
#include "controller.h"

#include <functional>
#include <string>
#include <vector>

#include <iostream>


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
     * Simple commands execute on the main debugger thread
     * asynchronously, and have no continuation in the UI.
     */
    template<typename Callable = std::function<void ()> >
    class SimpleCommand : public Command
    {
        Callable c_;
        bool done_;

    public:                
        SimpleCommand(Callable c) : c_(c), done_(false) { }
        ~SimpleCommand() throw() {}

        void execute_on_main_thread() 
        {
            c_();
            done_ = true;
        #if DEBUG
            std::clog << __func__ << std::endl;
        #endif
        }

        bool is_done() const
        {
            return done_;
        }
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
            return new SimpleCommand<Callable>(callable_);
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
