#ifndef CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89
#define CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "view.h"
#include <string>
#include <unordered_map>


namespace ui
{
    class Controller;


    class CodeView : public View
    {
    public:
        explicit CodeView(Controller&);
        
        virtual void update(const State&);

    protected:
        ~CodeView() throw();
    };

    typedef RefPtr<CodeView> CodeViewPtr;

/* would such a further specialization be useful?
    class SourceView : public CodeView
    {
    public:
    };

    class AsmView : public CodeView
    {
    public:
    };
*/

    /**
     * Composite code view.
     */
    class MultiCodeView : public CodeView
    {
    public:
        explicit MultiCodeView(Controller&);

    protected:
        ~MultiCodeView() throw();

        virtual void update(const State&);

        virtual CodeViewPtr make_view(const Symbol&) = 0;

    private:
        // map code views by file name
        std::unordered_map<SharedStringPtr, CodeViewPtr> views_;
    };
};

#endif // CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89

