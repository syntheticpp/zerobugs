#ifndef DELEGATE_H__B29137B9_0553_48F9_A9EC_EB9C94F2FC5F
#define DELEGATE_H__B29137B9_0553_48F9_A9EC_EB9C94F2FC5F

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <dwarf.h>
#include "interface.h"
#include "type.h"

namespace Dwarf
{
    /**
     * D Language extension
     */
    CLASS Delegate : public Type
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = 0x43 }; // see http://www.digitalmars.com/d/2.0/abi.html

        std::shared_ptr<Type> function_type() const;
        std::shared_ptr<Type> this_type() const;

    protected:
        Delegate(Dwarf_Debug, Dwarf_Die);
    };
}
#endif // DELEGATE_H__B29137B9_0553_48F9_A9EC_EB9C94F2FC5F
