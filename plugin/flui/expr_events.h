#ifndef EXPR_EVENTS_H__16B81AB4_8E69_43B0_8ED8_44DD5209C437
#define EXPR_EVENTS_H__16B81AB4_8E69_43B0_8ED8_44DD5209C437
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/debug_sym.h"
#include "zdk/expr.h"
#include "zdk/observer_impl.h"
#include "zdk/variant.h"


namespace ui
{
    class Controller;

    /**
     * Handle notifications from the expression evaluator.
     */
    class ExprEvalEvents : public SubjectImpl<ExprEvents>
    {
    public:
        static RefPtr<ExprEvalEvents> make(
            Controller&         controller,
            DebugSymbolEvents*  symEvents ) {

            return new ExprEvalEvents(controller, symEvents);
        }

    protected:
        ExprEvalEvents (
            Controller&         controller,
            DebugSymbolEvents*  symEvents )

            : controller_(controller)
            , symEvents_(symEvents) {
        }

        ExprEvalEvents(const ExprEvalEvents& other)
            : controller_(other.controller_)
            , symEvents_(other.symEvents_) {
        }
        ~ExprEvalEvents() throw();

        BEGIN_INTERFACE_MAP(ExprEvalEvents)
            INTERFACE_ENTRY_INHERIT(SubjectImpl<ExprEvents>)
        END_INTERFACE_MAP()


        /**
         * Indicates that the interpreter has finished parsing and
         * evaluating. When this notification is received, it is up
         * to the implementation whether to continue silently, or
         * enter interactive mode (prompt).
         *
         * @return true to enter interactive mode
         */
        virtual bool on_done(
            Variant*            variant,
            bool*               interactive,
            DebugSymbolEvents*  /* symEvents */)
        {
            if (variant)
            {
                if (auto sym = variant->debug_symbol())
                {
                    symEvents_->notify(sym);
                }
            }
            return true;
        }

        /**
         * An error occurred while interpreting expression
         */
        virtual void on_error(const char* msg);

        virtual void on_warning(const char* msg);

        /**
         * An event occurred on thread while interpreting expression
         * (i.e. a signal was raised, and it was not purposely caused
         * by the interpreter).
         * @return true if handled
         */
        virtual bool on_event(Thread*, Platform::addr_t);

        /**
         * The interpreter calls a function inside the debugged program.
         * @param retAddr return address of function
         * @param symbol if not NULL, the interpreter is about to call
         * the function of the corresponding symbol table entry; if the
         * symbol is NULL, the function has returned.
         */
        virtual void on_call(Platform::addr_t retAddr, Symbol* = nullptr);

        virtual ExprEvents* clone() const {
            return new ExprEvalEvents(*this);
        }

    private:
        Controller&         controller_;
        DebugSymbolEvents*  symEvents_;
    };
}

#endif // EXPR_EVENTS_H__16B81AB4_8E69_43B0_8ED8_44DD5209C437

