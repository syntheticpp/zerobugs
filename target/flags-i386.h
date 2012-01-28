#ifndef FLAGS_386_H__7E6B4DA6_AF2D_4DEB_B250_9569F563E881
#define FLAGS_386_H__7E6B4DA6_AF2D_4DEB_B250_9569F563E881
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

#include "reg.h"


class Flags386 : public Reg<reg_t>
{
public:
    DECLARE_UUID("d497625c-20d3-46f2-a7b5-b33b1b118fa3")

BEGIN_INTERFACE_MAP(Flags386)
    INTERFACE_ENTRY_INHERIT(Reg<reg_t>)
    INTERFACE_ENTRY(Flags386)
END_INTERFACE_MAP()

    Flags386(const char* name, const Thread& t, size_t off, reg_t val)
        : Reg<reg_t>(name, t, off, val) { }

    size_t
    enum_fields(EnumCallback3<const char*, reg_t, reg_t>*) const;

    bool set_value(const char* value, const char* name);
};


#endif // FLAGS_386_H__7E6B4DA6_AF2D_4DEB_B250_9569F563E881
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
