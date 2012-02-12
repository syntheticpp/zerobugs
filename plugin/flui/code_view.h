#ifndef CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89
#define CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/shared_string.h"
#include "view.h"
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

        virtual ViewType type() const { return VIEW_Code; }
    };

    typedef RefPtr<CodeView> CodeViewPtr;


    class SourceView : public CodeView
    {
    public:
        explicit SourceView(Controller& c) : CodeView(c) { }
    };

    class AsmView : public CodeView
    {
    public:
        explicit AsmView(Controller& c) : CodeView(c) { }
    };


    /**
     * Composite code view.
     */
    class MultiCodeView : public CodeView
    {
    public:
        explicit MultiCodeView(Controller&);

    protected:
        ~MultiCodeView() throw();

        virtual ViewType type() const { return VIEW_Code; }

        virtual void update(const State&);

    private:
        virtual Layout::CallbackPtr make_callback() = 0;

        virtual CodeViewPtr make_view(const Symbol&) = 0;

        // map code views by file name
        std::unordered_map<SharedStringPtr, CodeViewPtr> views_;
    };
};

#endif // CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89

