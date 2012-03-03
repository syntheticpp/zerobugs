#ifndef VIEW_H__1C73A6F4_AE29_4B8D_A06E_24BD9FD30117
#define VIEW_H__1C73A6F4_AE29_4B8D_A06E_24BD9FD30117
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
// Classes: State, View, Layout
//
#include "zdk/config.h"
#include "zdk/event_type.h"
#include "zdk/ref_counted_impl.h"
#include "zdk/ref_ptr.h"
#include "zdk/zerofwd.h"
#include "view_type.h"
#include <assert.h>
#include <vector>


namespace ui
{
    class Controller;
    class Layout;
    class View;

    typedef RefPtr<View> ViewPtr;


    /**
     * Models the current state of the debugger and UI.
     *
     * Updated on the main debugger thread when debug events
     * or other notifications are received.
     */
    struct State
    {
        virtual ~State() { }
        
        /** 
         * @note: this method is expected to be called on the
         * main debugger thread only
         */
        virtual void update(Thread*, EventType) = 0;

        virtual bool is_target_running() const = 0;
        /**
         * @return true if target is stopped in the debugger
         */
        virtual bool is_target_stopped() const = 0;

        virtual EventType current_event() const = 0;

        virtual RefPtr<Symbol> current_symbol() const = 0;

        virtual RefPtr<Thread> current_thread() const = 0;
    };

    /**
     * Abstract functor used during the construction of the UI layout.
     */
    struct LayoutCallback
    {
        virtual void insert() = 0;
    };

    /**
     * Base class for a view of the debug target.
     * Derived classes implement source code views,
     * CPU register views, call stack views, and so on.
     * It is reference counted so that it can be used
     * with a composite pattern.
     */
    class View : public RefCountedImpl<>
    {
        View(const View&); // non-copyable
        View& operator=(const View&);

    public:
        explicit View (Controller&);

        virtual ViewType type() const = 0;
        
        virtual void insert_self(LayoutCallback&) = 0;
        virtual void update(const State&) = 0;

        Controller& controller() const {
            return c_;
        }

        void awaken_main_thread();

        virtual void resize(int x, int y, int w, int h) = 0;

    protected:
        virtual ~View() throw() { }

    private:
        Controller& c_;
    };


    /**
     * Composite view that manages the layout of other views.
     */
    class Layout : public View
    {
    public:
        typedef std::unique_ptr<LayoutCallback> CallbackPtr;

        explicit Layout(Controller&);

        // template method pattern
        void add(View& v)
        {
            CallbackPtr cb(make_callback(v.type()));
            assert(cb);
            add(v, *cb);
        }

        virtual void show(View&, bool) = 0;
        virtual void status_message(const std::string&) = 0;

        virtual void update(const State&);

        virtual ViewType type() const
        {
            return VIEW_Layout;
        }

    protected:
        virtual ~Layout() throw() { }
        
        void add(View&, LayoutCallback&);
        
        virtual CallbackPtr make_callback(ViewType) = 0;
        
        virtual void insert_self(LayoutCallback&)
        {
        }

    private:
        std::vector<ViewPtr> views_;
    };

} // namespace

#endif // VIEW_H__1C73A6F4_AE29_4B8D_A06E_24BD9FD30117

