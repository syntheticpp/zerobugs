#ifndef RIGHT_CLICK_CONTEXT_H__7FAFF24A_93AF_4900_8503_BDBFDF00DAD6
#define RIGHT_CLICK_CONTEXT_H__7FAFF24A_93AF_4900_8503_BDBFDF00DAD6
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "menu_click.h"
#include "right_click.h"


class CodeView;


class ZDK_LOCAL RightClickContext : public MenuClickContext
{
public:
    explicit RightClickContext(CodeView&);

    ~RightClickContext() throw() { }

private:
    virtual Thread* thread() const;

    virtual size_t position() const;

private:
    CodeView& view_;
};

#endif // RIGHT_CLICK_CONTEXT_H__7FAFF24A_93AF_4900_8503_BDBFDF00DAD6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
