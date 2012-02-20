//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "controller.h"
#include "flvar_view.h"
#include "var_view.h"


ui::VarView::VarView(ui::Controller& c) : View(c)
{
}


ui::VarView::~VarView() throw()
{
}


void ui::VarView::update(const ui::State& s)
{
    variables_.clear();
    current_ = s.current_symbol();
}


bool ui::VarView::is_expanding(DebugSymbol* sym) const
{
    return sym && scope()->is_expanding(*sym);
}


void ui::VarView::expand(size_t row, bool flag)
{
    scope()->expand(get_variable(row), flag);
    awaken_main_thread();
}


bool ui::VarView::notify(DebugSymbol* sym)
{
    if (sym)
    {
        variables_.push_back(sym);

        if (is_expanding(sym))
        {
            sym->read(this);
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


RefPtr<ui::VarView::Scope> ui::VarView::scope() const
{
    if (RefPtr<Frame> f = controller().selection())
    {
        // ensure that there's scope object
        // associated with the stack frame object
        RefPtr<ZObject> obj = f->get_user_object(".scope");
        if (!obj)
        {
            obj = new ui::VarView::Scope;
            f->set_user_object(".scope", obj.get());
        }
        // change to the frame's scope if necessary
        if (!is_same_scope(f->function()))
        {
            scope_ = interface_cast<ui::VarView::Scope>(obj);
        }
    }

    if (!scope_)
    {
        scope_ = new ui::VarView::Scope;
    }
    return scope_;
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


