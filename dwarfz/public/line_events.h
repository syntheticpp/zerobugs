#ifndef LINE_EVENTS_H__9B72CBDC_0691_4679_A0C5_73EC4D37F12D
#define LINE_EVENTS_H__9B72CBDC_0691_4679_A0C5_73EC4D37F12D
//
// $Id: line_events.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <libdwarf.h>
#include "zdk/shared_string.h"


namespace Dwarf
{
    /**
     * Interface that receives source-line notifications
     * @see Debug::line_to_addr
     */
    struct SrcLineEvents
    {
        virtual ~SrcLineEvents() { }

        virtual bool on_srcline(
            SharedString*       file,
            Dwarf_Unsigned      line,
            Dwarf_Addr          addr) = 0;

        virtual void on_done() = 0;
    };
}
#endif // LINE_EVENTS_H__9B72CBDC_0691_4679_A0C5_73EC4D37F12D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
