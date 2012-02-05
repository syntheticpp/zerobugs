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
#include "zdk/interface_cast.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "properties.h"
#include "generic/lock.h"

using namespace std;


PropertiesImpl::PropertiesImpl()
{
}



PropertiesImpl::~PropertiesImpl() throw()
{
}



word_t PropertiesImpl::get_word(const char* name, word_t defaultVal)
{
    assert(name);
    Lock<Mutex> lock(mutex_);
    WordMap::iterator i = wordMap_.find(name);
    if (i == wordMap_.end())
    {
        return defaultVal;
    }
    return i->second;
}



void PropertiesImpl::set_word(const char* name, word_t val)
{
    assert(name);
    Lock<Mutex> lock(mutex_);

    wordMap_[name] = val;
}



double PropertiesImpl::get_double(const char* name, double defaultVal)
{
    assert(name);
    Lock<Mutex> lock(mutex_);
    DoubleMap::iterator i = doubleMap_.find(name);
    if (i == doubleMap_.end())
    {
        return defaultVal;
    }
    return i->second;
}



void PropertiesImpl::set_double(const char* name, double value)
{
    assert(name);
    Lock<Mutex> lock(mutex_);
    doubleMap_[name] = value;
}



const char* PropertiesImpl::get_string(const char* name, const char* defaultVal)
{
    assert(name);
    Lock<Mutex> lock(mutex_);
    if (!defaultVal)
    {
        defaultVal = "";
    }

    StringMap::iterator i = stringMap_.find(name);
    if (i == stringMap_.end())
    {
        return defaultVal;
    }
    return i->second.c_str();
}



void PropertiesImpl::set_string(const char* name, const char* value)
{
    assert(name);

    Lock<Mutex> lock(mutex_);
    if (!value)
    {
        stringMap_.erase(name);
    }
    else
    {
        stringMap_[name] = value;
    }
}



ZObject* PropertiesImpl::get_object(const char* name) const
{
    Lock<Mutex> lock(mutex_);
    ObjectMap::const_iterator i = objectMap_.find(name);

    if (i != objectMap_.end())
    {
        return i->second.get();
    }
    return NULL;
}



void PropertiesImpl::set_object(const char* name, ZObject* object)
{
    Lock<Mutex> lock(mutex_);
    ObjectMap::iterator i = objectMap_.find(name);

    if (i == objectMap_.end())
    {
        if (object)
        {
            objectMap_.insert(make_pair(name, object));
        }
    }
    else
    {
        if (!object)
        {
            objectMap_.erase(i);
            assert(objectMap_.find(name) == objectMap_.end());

            return;
        }

        // make sure that the objects are of the same type
        Streamable* objOld = interface_cast<Streamable*>(i->second.get());
        Streamable* objNew = interface_cast<Streamable*>(object);

        if (!objOld != !objNew)
        {
            string err = string(__func__) + ": type override: " + name;
            throw logic_error(err);
        }

        if (objOld && objNew && !uuid_equal(objOld->uuid(), objNew->uuid()))
        {
            ostringstream err;

            err << string(__func__) << ": type override: " << name << endl;
            err << "(" << *objOld->uuid() << " with " << *objNew->uuid() << ")";

            throw logic_error(err.str());
        }

        i->second = object;
    }
}



/**
 * Serialize properties to a stream
 */
size_t PropertiesImpl::write(OutputStream* os) const
{
    Lock<Mutex> lock(mutex_);

    assert(os);
    size_t bytesWritten = 0;
    // save all integers
    WordMap::const_iterator i = wordMap_.begin();
    for (; i != wordMap_.end(); ++i)
    {
        bytesWritten += os->write_word(i->first.c_str(), i->second);
    }
    // save all floats
    DoubleMap::const_iterator j = doubleMap_.begin();
    for (; j != doubleMap_.end(); ++j)
    {
        bytesWritten += os->write_double(j->first.c_str(), j->second);
    }
    // save strings
    StringMap::const_iterator k = stringMap_.begin();
    for (; k != stringMap_.end(); ++k)
    {
        const string& name = k->first;
        const string& value = k->second;
        bytesWritten += os->write_string(name.c_str(), value.c_str());
    }
    // save objects
    ObjectMap::const_iterator w = objectMap_.begin();
    for (; w != objectMap_.end(); ++w)
    {
        ZObject* object = w->second.get();
        if (!object)
        {
            continue;
        }

        Streamable* streamable = interface_cast<Streamable*>(object);
        if (streamable)
        {
            assert(interface_cast<InputStreamEvents*>(object));

            bytesWritten += os->write_object(w->first.c_str(), streamable);
        }
        else
        {
            cerr << "*** Warning: `" << w->first << "' is not streamable\n";
        }
    }
    return bytesWritten;
}


template<typename T>
static void merge(const T& src, T& dest)
{
    for (typename T::const_iterator i = src.begin(); i != src.end(); ++i)
    {
        dest[i->first] = i->second;
    }
}


void PropertiesImpl::merge(const PropertiesImpl& other)
{
    Lock<Mutex> lock(mutex_);

    ::merge(other.wordMap_, wordMap_);
    ::merge(other.doubleMap_, doubleMap_);
    ::merge(other.stringMap_, stringMap_);
    ::merge(other.objectMap_, objectMap_);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
