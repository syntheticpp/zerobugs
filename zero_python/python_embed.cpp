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
#if DEBUG
 #include <iostream>
#endif
#include "handle_error.h"
#include "python_embed.h"

using namespace std;
using namespace boost::python;


PythonEmbed::PythonEmbed()
{
    Py_Initialize();
    // PyEval_InitThreads();

    // get the main module
    object mainMod = object(handle<>(borrowed(PyImport_AddModule("__main__"))));

    // get the main namespace
    mainNS_ = dict(mainMod.attr("__dict__"));
    localNS_ = mainNS_;
}


PythonEmbed::~PythonEmbed()
{
    Py_Finalize();
}



boost::python::handle<> PythonEmbed::run_string(const string& cmd)
{
    handle<> result;

    try
    {
        PyObject* pyObj = PyRun_String(const_cast<char*>(cmd.c_str()),
                                       Py_single_input,
                                       mainNS_.ptr(),
                                       mainNS_.ptr());
        result = handle<>(pyObj);
    }
    catch (const error_already_set&)
    {
         python_handle_error();
    }
    return result;
}



handle<> PythonEmbed::run_file(FILE* fp, const string& filename)
{
    handle<> result;
    try
    {
        rewind(fp);
        result=handle<>(PyRun_File(fp,
                                   // const_cast for Py 2.2
                                   const_cast<char*>(filename.c_str()),
                                   Py_file_input,
                                   mainNS_.ptr(),
                                   localNS_.ptr()));
    }
    catch (const error_already_set&)
    {
        python_handle_error();
    }
    return result;
}



handle<> PythonEmbed::run_interactive(FILE* fp)
{
    return handle<>(PyRun_File( fp,
                                "<console>",
                                Py_single_input,
                                mainNS_.ptr(),
                                localNS_.ptr()));
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
