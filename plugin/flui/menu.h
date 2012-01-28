#ifndef MENU_H__131240E4_5274_4C57_8206_9572CF1CE829
#define MENU_H__131240E4_5274_4C57_8206_9572CF1CE829
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/ref_counted_impl.h"
#include "user_interface.h"

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
        { }

        explicit Menu(const std::string& name) : name_(name)
        { }

        ~Menu() throw()
        { }

        const std::string& name() const
        {
            return name_;
        }

        virtual void update(const State& state)
        { }

    private:
        std::string     name_;
    };



    class MenuItem : public Menu
    {
    public:
        //todo: shortcut?
        explicit MenuItem(const std::string& name) : Menu(name)
        { }

        ~MenuItem() throw()
        { }

        virtual std::unique_ptr<Command> emit() const;
    };



    class CompositeMenu : public Menu
    {
    public:
        explicit CompositeMenu(ui::Controller& c)
            : controller_(c)
        { }

        explicit CompositeMenu(
            ui::Controller&     c,
            const std::string&  name
          ) : Menu(name)
            , controller_(c)
        { }

        ~CompositeMenu() throw()
        { }
        /*
        virtual void add(RefPtr<MenuItem> menu) 
        {
            children_.push_back(menu);
        } */
        
        virtual void add(RefPtr<Menu> menu) 
        {
            children_.push_back(menu);
        }
        

    private:
        virtual void update(const State& state) 
        {
            for (auto i = children_.begin(); i != children_.end(); ++i)
            {
                (*i)->update(state);
            }
        }

        ui::Controller&             controller_;
        std::vector<RefPtr<Menu> >  children_;
    };
}

#endif // MENU_H__131240E4_5274_4C57_8206_9572CF1CE829
