// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <boost/python.hpp>
#include "zdk/get_pointer.h"
#include "zdk/stack.h"
#include "locked.h"
#include "stack_trace.h"

using namespace boost::python;


static Symbol* function(Frame* frame)
{
    return frame->function();
}


boost::python::list stack_frames(StackTrace* trace)
{
    boost::python::list frames;
    for (size_t i(0); i != trace->size(); ++i)
    {
        frames.append(RefPtr<Frame>(trace->frame(i)));
    }
    return frames;
}


void export_stack_trace()
{
    class_<Frame, bases<>, boost::noncopyable>("Frame", no_init)
        .def("program_count", &Frame::program_count)
        .def("frame_pointer", &Frame::frame_pointer)
        .def("stack_pointer", &Frame::stack_pointer)
        .def("index", &Frame::index)
        .def("is_signal_handler", &Frame::is_signal_handler)
        .def("function", function,
            "return the function in scope at this frame",
            return_value_policy<reference_existing_object>()
            )
        .def("symbol", function,
            "return the function in scope at this frame",
            return_value_policy<reference_existing_object>()
            )
        ;

    class_<StackTrace, bases<>, boost::noncopyable>("StackTrace", no_init)
        .def("size", &StackTrace::size,
            "return the number of frames in this stack trace",
            locked<>()
            )
        .def("frame", &StackTrace::frame,
            "return frame of specified index",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("frames", stack_frames,
            "return a the list of stack frame"
            )
        .def("select_frame", &StackTrace::select_frame,
            "select frame by index", locked<>()
            )
        .def("selection", &StackTrace::selection,
            "", locked<return_value_policy<reference_existing_object> >()
            )
        ;

    register_ptr_to_python<RefPtr<Frame> >();
    register_ptr_to_python<RefPtr<StackTrace> >();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
