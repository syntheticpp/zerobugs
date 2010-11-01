#ifndef INTERP_H__5EA5F6BD_C1B5_4D18_A2CE_182B71D56D8F
#define INTERP_H__5EA5F6BD_C1B5_4D18_A2CE_182B71D56D8F
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: interp.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <string>
#include "zdk/interp.h"
#include "zdk/weak_ptr.h"
#include "zdk/zobject_impl.h"
#include "zero_python/python_embed.h"

class Properties;


CLASS Python : public PythonEmbed, public ZObjectImpl<Interpreter>
{
    std::string         code_;
    RefPtr<ZObject>     context_;
    WeakPtr<Output>     output_;
    RefPtr<Properties>  license_;

public:
    Python();
    virtual ~Python() throw();

    const char* name() const;
    const char* lang_name() const;

    void run(Thread*, const char* cmd, Output*);

    void set_context(ZObject* context)
    {
        context_ = context;
    }

    void run_command(RefPtr<Thread>, std::string);
};


#endif // INTERP_H__5EA5F6BD_C1B5_4D18_A2CE_182B71D56D8F
