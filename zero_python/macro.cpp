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
#include "zdk/get_pointer.h"
#include "zdk/zero.h"
#include "dharma/properties.h"
#include "macro.h"
#include "marshaller.h"
#include "utility.h"

using namespace std;
using namespace boost;
using namespace boost::python;


Macro::Macro
(
    const char* name,
    PyObject* callable,
    PyObject* dict
)
  : callable_(callable)
  , prop_(new PropertiesImpl)
  , dict_(handle<>(PyDict_Copy(dict)))
{
    if (name)
    {
        name_ = name;
    }
    python_dict_to_properties(dict_, *prop_);
}



Macro::~Macro() throw()
{
}



const char* Macro::name() const
{
    return name_.c_str();
}


template<typename F>
inline RefPtr<SyncMessageAdapter<F> > adapt(const F& fun, const char* name)
{
    return new SyncMessageAdapter<F>(fun, name);
}

static bool call_command
(
    PyObject* callable,
    RefPtr<Thread> thread,
    boost::python::list argv
)
{
    bool result = false;
    try
    {
        result = call<bool>(callable, thread, argv);
    }
    catch (...)
    {
        throw runtime_error(python_get_error());
    }

    return result;
}



bool Macro::execute
(
    Thread* thread,
    const char* const* argv, // NULL-ended list of args
    Unknown2*                // not used
)
{
    bool result = false;

    if (callable_)
    {
        boost::python::list args;

        for (; argv && *argv; ++argv)
        {
            args.append(*argv);
        }

        //
        // execute the Python code on the event thread
        //
        //result = ThreadMarshaller::instance().send_event(
        result = ThreadMarshaller::instance().send_command(
                        bind(call_command, callable_, thread, args),
                        "execute-macro");
    }

    return result;
}



const char* Macro::help() const
{
/*
    if (PyObject* obj = PyDict_GetItemString(dict_.ptr(), "help"))
    {
        return PyString_AS_STRING(obj);
    }
 */
    const char* help = 0;
    if (prop_)
    {
        help = prop_->get_string("help");
    }
    return help ? help : "no help available";
}



void
Macro::auto_complete(const char*, EnumCallback<const char*>*) const
{
    // todo
}




void export_macro()
{
    class_<DebuggerCommand, bases<>, noncopyable>("DebuggerCommand", no_init)
        .def("name", &DebuggerCommand::name)
        ;

    class_<Macro, bases<DebuggerCommand>, noncopyable>("Command", no_init)
        ;

    register_ptr_to_python<RefPtr<Macro> >();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
