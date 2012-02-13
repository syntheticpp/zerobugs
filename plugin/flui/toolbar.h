#ifndef TOOLBAR_H__F25F4636_7424_4746_9873_E9D416868821
#define TOOLBAR_H__F25F4636_7424_4746_9873_E9D416868821
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "command.h"
#include "view.h"


namespace ui
{
    class Toolbar : public View
    {
    protected:
        explicit Toolbar(Controller&);
        virtual ~Toolbar() throw();

        ViewType type() const {
            return VIEW_Toolbar;
        }

    public:
        virtual void add_button(
            const char*         tooltip,
            const char* const*  pixmap,
            CommandPtr          command) = 0;

        template<typename F>
        void add_button(const char* tip, const char* const*  pix, const F& f)
        {
            add_button(tip, pix, new MainThreadCommand<F>(f));
        }

        void update(const State&) {
            // TODO
        }
    };
}

#endif // TOOLBAR_H__F25F4636_7424_4746_9873_E9D416868821

