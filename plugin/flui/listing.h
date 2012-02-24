#ifndef LISTING_H__B971556A_0FBE_4824_84CB_B16B1FD5F97F
#define LISTING_H__B971556A_0FBE_4824_84CB_B16B1FD5F97F
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/platform.h"
#include "zdk/ref_ptr.h"

struct Symbol;
struct Thread;


namespace ui
{
    using Platform::addr_t;

    /**
     * Model the notion of a "listing" which consists of lines
     * of code (can be high-level source, disassembly).
     */
    struct CodeListing
    {
        virtual ~CodeListing() { }
        
        virtual int addr_to_line(Platform::addr_t) const {
            return 0;
        }

        /** 
         * Update so that it contains the given symbol.
         * @return true if changed, false otherwise.
         */
        virtual bool refresh(
            const RefPtr<Thread>&,
            const RefPtr<Symbol>&) = 0;

        virtual const char* current_file() const = 0;

        virtual size_t current_line() const = 0;

        virtual size_t line_count() const = 0;

        virtual const std::string& line(size_t index) const = 0;
        
        virtual addr_t selected_addr() const = 0;
        virtual int selected_line() const = 0;
    };
}

#endif // LISTING_H__B971556A_0FBE_4824_84CB_B16B1FD5F97F

