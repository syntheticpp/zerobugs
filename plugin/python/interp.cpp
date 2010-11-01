//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: interp.cpp 714 2010-10-17 10:03:52Z root $
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
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include "interp.h"
#include "dharma/pipe.h"
#include "dharma/redirect.h"
#include "dharma/system_error.h"
#include "zdk/autocontext.h"
#include "zdk/shared_string_impl.h"
#include "zero_python/handle_error.h"
#include "zero_python/marshaller.h"
#include "zero_python/to_string.h"

using namespace std;
using namespace boost;
using namespace boost::python;


Python::Python()
{
}


Python::~Python() throw()
{
}


const char* Python::name() const
{
    return "Python Console";
}


const char* Python::lang_name() const
{
    return "python";
}


static inline string clear(string& s)
{
    string tmp;
    tmp.swap(s);

    return tmp;
}


//
// Read the output on a separate thread
//
static string result;

static void* read_output(void* p)
{
    try
    {
        result.clear();

        Pipe* pipe = static_cast<Pipe*>(p);

        char c = 0;
        for (int retryCount = 0; retryCount < 3; )
        {
            if (pipe->read(&c, 1, true))
            {
                result += c;
            }
            else
            {
                ++retryCount;
                usleep(100 * retryCount);
            }
        }
    }
    catch (const std::exception& e)
    {
        cerr << __func__ << ": " << e.what() << endl;
    }
    return 0;
}


void Python::run_command(RefPtr<Thread> thread, string code)
{
    try
    {
        Pipe pin, pout;

        // pipe our output to STDIN
        Redirect redirIn(STDIN_FILENO, pin.output());

        // pipe STDOUT to our input
        Redirect redirOut(STDOUT_FILENO, pout.input());

        pin.write(code);
        fsync(pin.input());

        AutoContext context(thread.get(), context_.get());

        try
        {
            run_interactive(stdin);
        }
        catch (const error_already_set&)
        {
            fflush(stdin);
            fflush(stdout);
            PyObject* exType = 0, *value = 0, *trace = 0;

            PyErr_Fetch(&exType, &value, &trace);
            // wrap PyObjects in handles so that we don't leak
            handle<> exc(allow_null(exType)),
                     val(allow_null(value)),
                     tr(allow_null(trace));

            // ignore IO errors, which may be caused by an interactive
            // command trying to read more data from the input pipe
            if (!PyErr_GivenExceptionMatches(exType, PyExc_IOError))
            {
                cout << python_get_error(exc, val, tr) << endl;
            }
        }

        pthread_t reader;
        if (pthread_create(&reader, 0, read_output, &pout) < 0)
        {
            throw SystemError(__func__);
        }
        pthread_join(reader, 0);
    }
    catch (const std::exception& e)
    {
        result = e.what();
    }

    if (RefPtr<Output> out = output_.lock())
    {
        out->print(result.c_str(), result.size());
    }
}


void Python::run(Thread* thread, const char* input, Output* output)
{
    bool run = true;

    if (input)
    {
        if (input[0])
        {
            code_ += input;

            if (isspace(input[0]))
            {
                code_ += "\n";
                run = false; // save it for later
            }
            else if (const size_t n = strlen(input))
            {
                if (input[n - 1] == ':')
                {
                    code_ += "\n";
                    run = false; // save it for later
                }
            }
        }
    }

    if (run && !code_.empty())
    {
        assert(ThreadMarshaller::instance().is_main_thread());
        output_ = output;

        string code = clear(code_);
        ThreadMarshaller::instance().send_event(
            bind(&Python::run_command, this, thread, code),
            __func__);
    }
}

