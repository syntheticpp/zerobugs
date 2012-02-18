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

    /**
     * Base (abstract) class for displaying some sort of CODE
     * listing (as source code, assembly, or whatever -- annotated?
     * format).
     */
    class CodeView : public View
    {
    public:
        // make the specified symbol (and the code surrounding it) visible
        virtual void show(RefPtr<Symbol>) = 0;

    protected:
        explicit CodeView(Controller&);
        ~CodeView() throw();

        virtual ViewType type() const { return VIEW_Code; }
    };

    typedef RefPtr<CodeView> CodeViewPtr;


    ////////////////////////////////////////////////////////////
    class SourceView : public CodeView
    {
    public:
        explicit SourceView(Controller& c) : CodeView(c) { }
       
    protected:
    };


    ////////////////////////////////////////////////////////////
    class AsmView : public CodeView
    {
    public:
        explicit AsmView(Controller& c) : CodeView(c) { }

    protected:
        virtual void show(RefPtr<Symbol>);

    private:
        
    };


    ////////////////////////////////////////////////////////////
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

        virtual void make_visible(CodeViewPtr) = 0;

    private:
        virtual Layout::CallbackPtr make_callback() = 0;

        virtual CodeViewPtr make_view(const Symbol&) = 0;

        // map code views by file name
        std::unordered_map<SharedStringPtr, CodeViewPtr> views_;
    };
};

#endif // CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89

