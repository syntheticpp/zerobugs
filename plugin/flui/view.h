#ifndef VIEW_H__1C73A6F4_AE29_4B8D_A06E_24BD9FD30117
#define VIEW_H__1C73A6F4_AE29_4B8D_A06E_24BD9FD30117
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/ref_counted_impl.h"
#include "zdk/stdexcept.h"
#include "zdk/zero.h"


namespace ui
{
    class Controller;
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

        virtual RefPtr<Symbol> current_symbol() const = 0;

        virtual RefPtr<Thread> current_thread() const = 0;
    };


    /**
     * Base class for a view of the debug target.
     * Derived classes implement source code views,
     * CPU register views, call stack views, and so on.
     * It is reference counted so that it can be used
     * in a composite pattern.
     */
    class View : public RefCountedImpl<>
    {
    public:
        explicit View (Controller&);

        virtual void added_to(const View&) = 0;
        virtual void update(const State&) = 0;

    protected:
        virtual ~View() throw() { }

        Controller& controller() const 
        {
            return c_;
        }

    private:
        Controller& c_;
    };

    typedef RefPtr<View> ViewPtr;

    
    /**
     * Composite view that manages the layout of other views.
     */
    class Layout : public View
    {
    public:
        explicit Layout(Controller&);

        virtual void add(View&);

        virtual void show(View&, bool) = 0;

        virtual void update(const State&);

    protected:
        virtual ~Layout() throw() { }
        
    private:
        std::vector<ViewPtr> views_;
    };
}

#endif // VIEW_H__1C73A6F4_AE29_4B8D_A06E_24BD9FD30117

