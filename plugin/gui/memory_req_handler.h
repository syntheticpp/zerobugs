#ifndef MEMORY_REQ_HANDLER_H__A659FCF6_1525_4216_9664_058862ABF7CA
#define MEMORY_REQ_HANDLER_H__A659FCF6_1525_4216_9664_058862ABF7CA
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
#include "gtkmm/base.h"
#include <boost/shared_ptr.hpp>
#include <deque>

struct MemoryRequest;
struct ProgramViewSignals;
class AppSlots;


/**
 * Handles requests for reading chunks of
 * debugged program's memory on the UI thread.
 *
 * Use case: a snippet of code has to be disassembled.
 *  A request to read the debugged program's memory is
 *  posted from the UI thread to the main thread. When
 *  it completes, a notification gets posted back to the
 *  UI thread.
 */
class MemoryRequestHandler : public Gtk::Base
{
    typedef std::deque<RefPtr<MemoryRequest> > MemoryRequests;

    AppSlots& marshaller_;
    MemoryRequests memRequests_;
    size_t reqPending_;
    boost::shared_ptr<ProgramViewSignals> progView_;

public:
    MemoryRequestHandler(AppSlots& marshaller,
                         boost::shared_ptr<ProgramViewSignals>);

    ~MemoryRequestHandler();

    void on_read_memory(RefPtr<MemoryRequest>);
    void on_read_done(RefPtr<MemoryRequest>);

private:
    void read_memory(RefPtr<MemoryRequest>) volatile;

    void unqueue_request();
};


#endif // MEMORY_REQ_HANDLER_H__A659FCF6_1525_4216_9664_058862ABF7CA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
