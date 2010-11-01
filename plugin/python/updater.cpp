//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: updater.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <boost/python.hpp>
#include "dharma/environ.h"
#include "dharma/system_error.h"
#include "generic/auto_file.h"
#include "updater.h"
#include "sysid.h"
#include "zero_python/call.h"
#include "zero_python/utility.h"

using namespace std;
using namespace boost;
using namespace boost::python;


Updater::Updater(const RefPtr<Python>& python)
    : python_(python)
{
}


size_t Updater::check_for_updates(EnumCallback<Update*>* callback)
{
    size_t count = 0;
    if (python_) try
    {
        std::string fileName = env::get_string("ZERO_PLUGIN_PATH") + "/update.py";
        auto_file f(fopen(fileName.c_str(), "r"));
        if (!f.is_valid())
        {
            throw SystemError(fileName);
        }
        python_->run_file(f.get(), fileName);
        if (object updates = call_<object>("check_for_updates", SYSID, BUILDTIME))
        {
            count = __len__(updates);
            for (size_t i = 0; i != count; ++i)
            {
                if (object obj = updates[i])
                {
                    Update& updateInfo = extract<Update&>(obj);
                    RefPtr<Update> u = updateInfo.copy();
                    if (callback)
                    {
                        callback->notify(u.get());
                    }
                }
            }
        }
    }
    catch (const error_already_set&)
    {
         throw runtime_error(python_get_error());
    }
    return count;
}
