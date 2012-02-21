//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/thread_util.h"
#include "controller.h"
#include "flvar_view.h"
#include "var_view.h"
#include <iostream>
using namespace std;


ui::VarView::VarView(ui::Controller& c) : View(c)
{
}


ui::VarView::~VarView() throw()
{
}


void ui::VarView::update(const ui::State& s)
{
    variables_.clear();

    RefPtr<Thread> thread = s.current_thread();

    if (thread && thread_finished(*thread))
    {
        current_.reset();
    }
    else if (RefPtr<Frame> f = thread_current_frame(thread.get()))
    {
        const bool sameScope = is_same_scope(f->function()); 
        current_ = f->function();

        if (sameScope && scope_)
        {
            return;
        }

        RefPtr<ZObject> obj = f->get_user_object(".scope");
        // ensure that there's scope object
        // associated with the stack frame object
        if (!obj)
        {
            obj = new ui::VarView::Scope;
            f->set_user_object(".scope", obj.get());
        }

        if (!sameScope)
        {
            // make it the current scope
            scope_ = interface_cast<ui::VarView::Scope>(obj);
        }
    }

    // make sure we always have a non-null scope
    if (!scope_)
    {
        scope_ = new ui::VarView::Scope;
    }
}


bool ui::VarView::is_expanding(DebugSymbol* sym) const
{
    return sym && scope().is_expanding(*sym);
}


void ui::VarView::expand(size_t row, bool flag)
{
    scope().expand(get_variable(row), flag);
    awaken_main_thread();
}


bool ui::VarView::notify(DebugSymbol* sym)
{
    if (sym)
    {
        variables_.push_back(sym);
        sym->read(this);

        if (is_expanding(sym))
        {
            sym->enum_children(this);
        }
    }
    return true;
}


DebugSymbol& ui::VarView::get_variable(size_t n) const
{
    if (n >= variables_.size())
    {
        throw std::out_of_range(__func__);
    }

    return *variables_[n];
}


bool ui::VarView::is_same_scope(Symbol* sym) const
{
    return current_ && sym && (current_->value() == sym->value());
}


ui::VarView::Scope& ui::VarView::scope() const
{
    assert(scope_);
    return *scope_;
}


bool ui::VarView::Scope::is_expanding(const DebugSymbol& sym) const
{
    SymKey key(sym);

    auto i = vars_.find(key);
    if (i != vars_.end())
    {
        return i->second.expand_;
    }
    return false;
}


void ui::VarView::Scope::expand(DebugSymbol& sym, bool expand)
{
    SymKey key(sym);
    vars_[key].expand_ = expand;
}


