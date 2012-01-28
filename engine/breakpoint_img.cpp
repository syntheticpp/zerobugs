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

#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include "zdk/breakpoint_util.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/switchable.h"
#include "breakpoint_img.h"

using namespace std;


BreakPointImage::BreakPointImage(volatile BreakPoint& bpnt)
    : Persistent("BreakPointImage")
    , addr_(bpnt.addr())
    , enabled_(has_enabled_actions(bpnt))
    , deferred_(bpnt.is_deferred())
    , line_(0)
    , activationCount_(0)
    , lwpid_(CHKPTR(bpnt.thread())->lwpid())
    , type_(bpnt.type())
{
    if (Symbol* symbol = bpnt.symbol())
    {
        symName_ = symbol->name();
        line_ = symbol->line();
        file_ = symbol->file();
    }
    const size_t count = bpnt.action_count();
    for (size_t i = 0; i != count; ++i)
    {
        const BreakPoint::Action* act = bpnt.action(i);
        if (const Switchable* sw = interface_cast<const Switchable*>(act))
        {
            condition_ = sw->activation_expr();
            activationCount_ = sw->activation_counter();

            if (sw->auto_reset())
            {
                static const word_t f = (1L << numeric_limits<word_t>::digits);
                BOOST_STATIC_ASSERT(f < 0);
                activationCount_ |= f;
            }
            // we should only have one user-defined action per
            // breakpoint, so we break out of the loop here
            break;
        }
    }
}


BreakPointImage::BreakPointImage()
    : Persistent("BreakPointImage")
    , addr_(0)
    , enabled_(-1)
    , deferred_(-1)
    , activationCount_(0)
    , lwpid_(0)
    , type_(BreakPoint::SOFTWARE)
{
}


BreakPointImage::~BreakPointImage() throw()
{
}


bool BreakPointImage::auto_reset() const
{
    return activationCount_ & (1L << numeric_limits<word_t>::digits);
}


word_t BreakPointImage::activation_counter() const
{
    return activationCount_ & ~(1L << numeric_limits<word_t>::digits);
}


void BreakPointImage::set_addr(addr_t addr)
{
    assert(addr_ == 0); // set once
    addr_ = addr;
}


void BreakPointImage::on_word(const char* name, word_t value)
{
    if (strcmp(name, "addr") == 0)
    {
        set_addr(value);
    }
    else if (strcmp(name, "enbl") == 0)
    {
        assert(enabled_ == -1);
        enabled_ = value;
    }
    else if (strcmp(name, "line") == 0)
    {
        line_ = value;
    }
    else if (strcmp(name, "actc") == 0)
    {
        assert(activationCount_ == 0);
        activationCount_ = value;
    }
    else if (strcmp(name, "type") == 0)
    {
        assert(type_ == BreakPoint::SOFTWARE);
        type_ = (BreakPoint::Type)value;
    }
    else if (strcmp(name, "lwpid") == 0)
    {
        assert(lwpid_ == 0);
        lwpid_ = value;
    }
    else if (strcmp(name, "defer") == 0)
    {
        assert(deferred_ == -1);
        deferred_ = value;
    }
    else
    {
        Persistent::on_word(name, value);
    }
}


void BreakPointImage::on_string(const char* name, const char* value)
{
    assert(value);

    if (strcmp(name, "sym") == 0)
    {
        assert(!symName_);
        symName_ = shared_string(value);
    }
    else if (strcmp(name, "file") == 0)
    {
        assert(!file_);
        file_ = shared_string(value);
    }
    else if (strcmp(name, "cond") == 0)
    {
        assert(condition_.empty());
        condition_ = value;
    }
    else
    {
        Persistent::on_string(name, value);
    }
}


size_t BreakPointImage::write(OutputStream* stream) const
{
    size_t nbytes = stream->write_word("addr", addr_);
    nbytes += stream->write_word("enbl", enabled_);
    nbytes += stream->write_word("defer", deferred_);

    if (line_)
    {
        nbytes += stream->write_word("line", line_);
    }
    if (symName_.get())
    {
        nbytes += stream->write_string("sym", symName_->c_str());
    }
    if (file_.get())
    {
        nbytes += stream->write_string("file", file_->c_str());
    }
    if (!condition_.empty())
    {
        nbytes += stream->write_string("cond", condition_.c_str());
    }
    if (activationCount_)
    {
        nbytes += stream->write_word("actc", activationCount_);
    }
    if (lwpid_)
    {
        nbytes += stream->write_word("lwpid", lwpid_);
    }
    nbytes += stream->write_word("type", type_);
    return nbytes;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
