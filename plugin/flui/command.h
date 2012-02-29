#ifndef COMMAND_H__924DE56A_4E47_4F94_A8B5_BF0BF6BEDEF2
#define COMMAND_H__924DE56A_4E47_4F94_A8B5_BF0BF6BEDEF2
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/ref_counted_impl.h"
#include "zdk/ref_ptr.h"
#include <functional>

namespace ui
{
    class Controller;

    /**
     * The UI typically runs an "event-pump" on a separate thread
     * that the main debugger thread (which attaches to targets
     * and manipulates them via ptrace; all ptrace calls have to
     * be issued by the main thread).
     * The UI may issue a "command" to the main thread, which will
     * execute it and possibly continue with another action on the
     * UI thread.
     * @see Controller::on_event for how this class is used.
     */
    class Command : public RefCountedImpl<>
    {
    protected:
        virtual ~Command() throw() { }

    public:
        // request from ui to main thread
        virtual void execute_on_main_thread() { }
        // response to ui thread
        virtual void continue_on_ui_thread(Controller&) { }

        virtual bool is_complete() const { return true; }

        // reset whatever state the implementation maintains internally
        virtual void reset() { }
    };


    /**
     * Models simple commands that execute on the main debugger
     * thread asynchronously and have no continuation in the UI.
     */
    template<typename Callable = std::function<void ()> >
    class MainThreadCommand : public Command
    {
        Callable    callable_;
        bool        complete_;

    public:
        MainThreadCommand(Callable callable)
            : callable_(callable)
            , complete_(false)
        { }

    protected:
        ~MainThreadCommand() throw() { }

        void execute_on_main_thread() {
            callable_();
            complete_ = true;
        }

        bool is_complete() const {
            return complete_;
        }

        void reset() {
            complete_ = false;
        }
    };


    template<typename Callable = std::function<void (Controller&)> >
    class UIThreadCommand : public Command
    {
        Callable    callable_;
        bool        complete_;

    public:
        UIThreadCommand(Callable callable)
            : callable_(callable)
            , complete_(false)
        { }

    protected:
        ~UIThreadCommand() throw() { }

        void reset() {
            complete_ = false;
        }

        void continue_on_ui_thread(Controller& controller) {
            callable_(controller);
            complete_ = true;
        }

        bool is_complete() const {
            return complete_;
        }
    };


    typedef RefPtr<Command> CommandPtr;
}
#endif // COMMAND_H__924DE56A_4E47_4F94_A8B5_BF0BF6BEDEF2

