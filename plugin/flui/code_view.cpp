//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/config.h"
#include "zdk/check_ptr.h"
#include "zdk/symbol.h"
#include "code_view.h"
#include "flcode_table.h"


ui::CodeView::CodeView(ui::Controller& controller)
    : ui::View(controller)
{
}


ui::CodeView::~CodeView() throw()
{
}


void ui::CodeView::update(const ui::State&)
{
    
}


ui::MultiCodeView::MultiCodeView(ui::Controller& controller)
    : ui::CodeView(controller)
{
}


ui::MultiCodeView::~MultiCodeView() throw()
{
}


void ui::MultiCodeView::update(const ui::State& s)
{
    if (RefPtr<Symbol> sym = s.current_symbol())
    {
        SharedStringPtr filename = sym->file();

        auto i = views_.find(filename);
    //
    // TODO: observe some upper limit?
    //
        if (i == views_.end())
        {
            CodeViewPtr cv = make_view(*sym);
            if (cv.is_null())
            {
                return;
            }
            i = views_.insert(std::make_pair(filename, cv)).first;

            Layout::CallbackPtr cb(make_callback());

            assert(cb);
            cv->insert_self(*cb);
        }

        CHKPTR(i->second)->update(s);
    }
}

