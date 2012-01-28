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
#include <assert.h>
#include <exception>
#include <iostream>
#include "zdk/align.h"
#include "zdk/zero.h"
#include "app_slots.h"
#include "assert_thread.h"
#include "command.h"
#include "memory_req.h"
#include "prog_view_signals.h"
#include "slot_macros.h"
#include "memory_req_handler.h"


using namespace boost;
using namespace std;



MemoryRequest::MemoryRequest
(
    RefPtr<Symbol>  sym,
    size_t          size,
    ViewType        type,
    bool            hilight,
    RefPtr<SharedString> filename
)
  : begin(sym)
  , requestedSize(size)
  , viewType(type)
  , hilite(hilight)
  , file(filename)
{
}


MemoryRequest::~MemoryRequest() throw()
{
}


MemoryRequestHandler::MemoryRequestHandler
(
    AppSlots& marshaller,
    boost::shared_ptr<ProgramViewSignals> progView
)
    : marshaller_(marshaller)
    , reqPending_(0)
    , progView_(progView)
{
}


MemoryRequestHandler::~MemoryRequestHandler()
{
}



BEGIN_SLOT(MemoryRequestHandler::on_read_memory,
(
    RefPtr<MemoryRequest> req
))
{
    assert_ui_thread();
    assert(req);

    if (reqPending_)
    {
    #ifdef DEBUG
        clog << "memory request pending, queued\n";
    #endif
        memRequests_.push_back(req); // queue it for later
    }
    else
    {
        ++reqPending_;

        CommandPtr cmd = command(&MemoryRequestHandler::read_memory, this, req);
        marshaller_.run_on_main_thread(cmd);
    }
}
END_SLOT()



void MemoryRequestHandler::read_memory(RefPtr<MemoryRequest> req) volatile
{
    assert(req);
    assert_main_thread();

    RefPtr<Thread> thread = marshaller_.current_thread();

    if (!thread)
    {
        cerr << "*** Warning: " << __func__ << ": NULL thread!\n";
        return;
    }
    assert(req->begin);

    const addr_t begin = req->begin->addr();

    try
    {
        const size_t nwords = round_to_word(req->requestedSize);

        req->bytes.resize(nwords * sizeof(word_t));
        word_t* buf = (word_t*) &((req->bytes)[0]);

        // todo: add a parameter to the MemoryRequest
        // that indicates whether to read CODE or DATA?
        // (for Linux and BSD it does not matter anyway)

        thread->read_code(begin, buf, nwords);
        req->requestedSize = req->bytes.size();
    }
    catch (const std::exception& e)
    {
        cerr << "*** Warning: " << e.what() << endl;
        cerr << __func__ << ": " << hex << begin << ", ";
        cerr << dec << req->requestedSize << " bytes\n";
    }

    marshaller_.update_stack_traces();
    //
    // queue on_read_done invocation for the code viewer object
    //
    CommandPtr cmd = command(&MemoryRequestHandler::on_read_done,
                             this,
                             req);
    marshaller_.post_request(cmd);
}



void
MemoryRequestHandler::unqueue_request()
{
    if (!memRequests_.empty())
    {
        RefPtr<MemoryRequest> req = memRequests_.front();
        memRequests_.pop_front();

    #ifdef DEBUG
        clog << "unqueued memory request\n";
    #endif

        on_read_memory(req);
    }
}


void
MemoryRequestHandler::on_read_done(RefPtr<MemoryRequest> req)
{
    assert_ui_thread();

    assert(reqPending_);
    --reqPending_;
    unqueue_request();

    if (progView_.get())
    {
        progView_->on_read_done(req);
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
