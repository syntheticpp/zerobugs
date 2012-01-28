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

#include <iostream>
#include <stdexcept>
#include "dharma/environ.h"
#include "generic/temporary.h"
#include "settings.h"

using namespace std;



Settings::Settings(WeakPtr<ObjectFactory> factory) : factory_(factory)
{
}


Settings::~Settings() throw()
{
}


void Settings::on_word(const char* name, word_t val)
{
    set_word(name, val);
}


void Settings::on_double(const char* name, double val)
{
    set_double(name, val);
}


void Settings::on_string(const char* name, const char* val)
{
    set_string(name, val);
}


size_t Settings::on_object_begin(
    InputStream*    stream,
    const char*     name,
    uuidref_t       uuid,
    size_t          size)
{
    assert(stream);

    Lock<Mutex> lock(mutex_);

    RefPtr<ObjectFactory> factory = factory_.ref_ptr();
    if (!factory)
    {
        clog << "*** Warning: " << __func__ << ": no factory in " << *_uuid() << endl;
        return 0;
    }

    if (RefPtr<ZObject> obj = factory->create_object(uuid, name))
    {
        InputStreamEvents& events = interface_cast<InputStreamEvents&>(*obj);

        size_t nread = 0;

        for (; nread < size; )
        {
            if (size_t nbytes = stream->read(&events))
            {
                nread += nbytes;
            }
            else
            {
                break;
            }
        }

        set_object(name, obj.get());
        events.on_object_end(); // signal that we are done reading the obj

        return nread;
    }
    return 0;
}


void Settings::on_opaque(
    const char* /* name */,
    uuidref_t   /* uuid */,
    const uint8_t* bytes,
    size_t size)
{
    Lock<Mutex> lock(mutex_);
    Opaque opaque(bytes, bytes + size);

    opaqueList_.push_back(opaque);
}


size_t Settings::write(OutputStream* stream) const
{
    assert(stream);
    Lock<Mutex> lock(mutex_);

    static const bool save = env::get_bool("ZERO_SAVE_STATE", true);
    if (!save)
    {
        return 0;
    }

    size_t count = PropertiesImpl::write(stream);

    OpaqueList::const_iterator i = opaqueList_.begin();

    for (; i != opaqueList_.end(); ++i)
    {
        count += stream->write_opaque(&(*i)[0], (*i).size());
    }
    return count;
}


void Settings::set_object(const char* name, ZObject* obj)
{
    PropertiesImpl::set_object(name, obj);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
