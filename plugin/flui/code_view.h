#ifndef CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89
#define CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "view.h"

namespace ui
{
    class Controller;


    class CodeView : public View
    {
    public:
        explicit CodeView(ui::Controller&);

    protected:
        ~CodeView() throw();

    private:
        virtual void update(const State&);
    };
/*
    class SourceView : public CodeView
    {
    public:
    };

    class AsmView : public CodeView
    {
    public:
    };
*/
    // composite
    class MultiCodeView : public CodeView
    {
    public:
    };
};
#endif // CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89
