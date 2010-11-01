#ifndef RIGHT_CLICK_H__83910AB7_2D53_4034_8AFC_AB7073ADBF48
#define RIGHT_CLICK_H__83910AB7_2D53_4034_8AFC_AB7073ADBF48
//
// $Id: right_click.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <math.h>   // labs
#include <vector>
#include "generic/lock.h"
#include "zdk/atomic.h"
#include "zdk/platform.h"
#include "zdk/mutex.h"


class ZDK_LOCAL RightClickInfo
{
public:
    typedef Platform::addr_t addr_t;
    typedef std::vector<addr_t> AddrList;
    typedef std::vector<size_t> LineList;
    typedef std::vector<RefPtr<Symbol> > SymbolList;

    RightClickInfo()
        : nearestAddr_(0)
        , pos_(0)
        , programCounter_(0)
    { }

    ~RightClickInfo() { }

    /**
     * @return the address in the binary that is the
     * nearest match for the click in the source code.
     */
    addr_t nearest_addr() const
    {
        return nearestAddr_;
    }

    const AddrList& addrs() const
    {
        return addrs_;
    }

    const SymbolList& deferred_symbols() const
    {
        return deferredSymbols_;
    }

    void set_program_count(addr_t pc) { programCounter_ = pc; }
    addr_t program_count() const { return programCounter_; }

    /**
     * used by CodeView::get_src_addr, CodeView::get_asm_addr
     * to populate addresses
     */
    void push_back(addr_t addr)
    {
        addrs_.push_back(addr);

        if (nearestAddr_ == 0 ||
            labs(addr - programCounter_) < labs(nearestAddr_ - programCounter_))
        {
            nearestAddr_ = addr;
        }
    }

    void add_deferred_symbol(const RefPtr<Symbol>& sym)
    {
        deferredSymbols_.push_back(sym);
    }

    void add_line(size_t line) { lines_.push_back(line); }
    const LineList& lines() const { return lines_; }

    size_t position() const { return pos_; }
    void set_position(size_t pos) { pos_ = pos; }

    void clear()
    {
        pos_ = nearestAddr_ = 0;
        programCounter_ = 0;
        addrs_.clear();
        lines_.clear();
        deferredSymbols_.clear();
    }

private:
    AddrList        addrs_;
    LineList        lines_;
    SymbolList      deferredSymbols_;
    Mutex           mutex_;
    addr_t          nearestAddr_;
    size_t          pos_;
    addr_t          programCounter_;
};

#endif // RIGHT_CLICK_H__83910AB7_2D53_4034_8AFC_AB7073ADBF48
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
