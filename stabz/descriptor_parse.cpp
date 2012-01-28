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
//
#include <iostream>
#include <stdexcept>
#include "dharma/environ.h"
#include "public/compile_unit.h"
#include "public/descriptor.h"
#include "public/fwdtype.h"

#include "private/init_events.h"
#include "private/parse_events.h"
#include "private/throw.h"
#include "zdk/shared_string_impl.h"

using namespace std;


static bool abort_stabz_parse()
{
#if DEBUG
    static bool flag = env::get_bool("ZERO_ABORT_STABZ_PARSE");
    return flag;
#endif
    return false;
}


void Stab::Descriptor::parse(TypeSystem& typeSystem)
{
    assert(!abort_stabz_parse());
    if (parsed_)
    {
        assert(initialized_);
        return;
    }
    else if (!initialized_)
    {
        init_and_parse(typeSystem);
    }
    else
    {
        Stab::ParseEvents parse(typeSystem, *this);

        Stab::Events* events[] = { &parse };

        for_each_stab_section(events, 1);

        end_parse();
    }
}


void Stab::Descriptor::init_and_parse(TypeSystem& typeSystem)
{
    if (parsed_)
    {
        return;  // already parsed, bail out
    }
    assert(!abort_stabz_parse());
    if (initialized_)
    {
        parse(typeSystem);
    }
    else
    {
        Stab::InitEvents init(*this);
        Stab::ParseEvents parse(typeSystem, *this);

        Stab::Events* events[] = { &init, &parse };

        initialized_ = true;

        for_each_stab_section(events, 2);

        end_parse();
    }
}


void Stab::Descriptor::end_parse()
{
    parsed_ = true;

    fwdTypeMap_.clear();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
