#ifndef PYTHON_EMBED_H__EE3759A2_51F7_4673_A0BE_9154FE43FBE2
#define PYTHON_EMBED_H__EE3759A2_51F7_4673_A0BE_9154FE43FBE2
//
// $Id: python_embed.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/export.h"
#include "generic/temporary.h"
#include <string>
#include <boost/python.hpp>
#include <boost/utility.hpp>


/**
 * Helper class for embedding a python interpreter.
 */
class ZDK_LOCAL PythonEmbed : private boost::noncopyable
{
public:
    PythonEmbed();
    ~PythonEmbed();

    boost::python::handle<> run_file(FILE*, const std::string&);
    boost::python::handle<> run_interactive(FILE*);

    boost::python::handle<> run_string(const std::string&);

    boost::python::object main_ns() const { return mainNS_; }

  /*
    template<typename T>
    T extract(const std::string& name) const
    {
        boost::python::extract<T> x(mainNS_[name]);
        if (x.check())
        {
            return x();
        }
        return T();
    } */

private:
    boost::python::dict mainNS_;
    boost::python::dict localNS_;
};


#endif // PYTHON_EMBED_H__EE3759A2_51F7_4673_A0BE_9154FE43FBE2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
