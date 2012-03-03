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
    class State;


    class Dialog
    {
    public:
        typedef std::vector<RefPtr<View> > Views;
        typedef std::function<void ()> Callback;
        typedef std::unordered_map<std::string, Callback> Actions;

        explicit Dialog(Controller&);
        virtual ~Dialog();

        virtual void close() = 0;

        void popup(const ui::State& s) {
            update(s);
            show();
        }

        virtual void show(bool = true) = 0;

        virtual void update(const ui::State&);

    protected:
        void add_action(const std::string&, Callback);

        void add_view(const RefPtr<View>& v) {
            views_.push_back(v);
        }

        Controller& controller() {
            return controller_;
        }

    private:
        // non-copyable
        Dialog(const Dialog&);
        Dialog& operator=(const Dialog&);

    private:
        Controller& controller_;
        Views       views_;
        Actions     actions_;
    };
}

#endif // DIALOG_H__A23FEB12_3003_40BC_9BCB_3ECCF55C6EE6

