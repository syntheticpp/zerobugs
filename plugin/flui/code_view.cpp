//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/config.h"
#include "zdk/check_ptr.h"
#include "zdk/zero.h"
#include "code_view.h"
#include "flcode_table.h"
#include <iostream>


ui::CodeView::CodeView(ui::Controller& controller)
    : ui::View(controller)
{
}


ui::CodeView::~CodeView() throw()
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
    RefPtr<Symbol> sym;

    RefPtr<Thread> t = s.current_thread();
    if (t)
    {
        RefPtr<Frame> f = t->stack_trace()->selection();
        if (f)
        {
        #if DEBUG
            std::clog << __func__ << ": selection=frame " << f->index() << std::endl;
        #endif

            sym = f->function();
        }
    }
#if 0
    if (!sym)
    {
        sym = s.current_symbol();
    }
#endif

    if (sym)
    {
    #if DEBUG
        std::clog << __func__ << ": " << sym->line() << std::endl;
    #endif

        SharedStringPtr filename = sym->file();

        auto i = views_.find(filename);
    //
    // TODO: observe some upper limit on how many views can be open at a given time?
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

        CHKPTR(i->second)->show(sym);
    }
    // TODO: else...?
}


void ui::AsmView::show(RefPtr<Symbol> sym)
{
}

