#ifndef DIALOG_H__A23FEB12_3003_40BC_9BCB_3ECCF55C6EE6
#define DIALOG_H__A23FEB12_3003_40BC_9BCB_3ECCF55C6EE6
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "view.h"
#include <functional>
#include <string>
#include <unordered_map>


namespace ui
{
    class Dialog
    {
    public:
        typedef std::vector<RefPtr<View> > Views;
        typedef std::function<void ()> Callback;
        typedef std::unordered_map<std::string, Callback> Actions;

        explicit Dialog(Controller&);
        virtual ~Dialog();

        virtual void show_modal() { }

    protected:
        virtual void add_action(const std::string&, Callback);
        virtual void add_view(const RefPtr<View>&) { }

    private:

    private:
        Controller& controller_;
        Views       views_;
        Actions     actions_;
    };
}

#endif // DIALOG_H__A23FEB12_3003_40BC_9BCB_3ECCF55C6EE6

