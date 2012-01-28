#ifndef VIEW_H__1C73A6F4_AE29_4B8D_A06E_24BD9FD30117
#define VIEW_H__1C73A6F4_AE29_4B8D_A06E_24BD9FD30117
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/stdexcept.h"
#include "zdk/zero.h"


namespace ui
{
    class Layout;

    /**
     * Models the current state of the debugger and UI.
     */
    struct State
    {
        virtual ~State() { }

        /** 
         * NOTE: this method is expected to be called on the
         * main debugger thread only
         */
        virtual void update(Thread*, EventType) = 0;

        /**
         * @return true if target is stopped in the debugger
         */
        virtual bool is_target_stopped() const = 0;

        virtual void set_target_stopped(bool) = 0;

        virtual EventType current_event_type() const = 0;
        virtual Thread* current_thread() const = 0;
    };


    class View
    {
    public:
        virtual ~View() { }

        virtual void update(const State&) = 0;

        // visitor pattern
        virtual void accept(Layout&) = 0;
    };

    
    /**
     * Composite view that manages the layout of other views.
     */
    class Layout : public View
    {
    public:
        virtual void add(View&) = 0;
        virtual void show(View&, bool) = 0;
    };


}
#endif // VIEW_H__1C73A6F4_AE29_4B8D_A06E_24BD9FD30117
