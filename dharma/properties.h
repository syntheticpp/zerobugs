#ifndef PROPERTIES_H__8D476F2F_5AD8_4A8B_A075_3BDA6D0FF48F
#define PROPERTIES_H__8D476F2F_5AD8_4A8B_A075_3BDA6D0FF48F
//
// $Id: properties.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <map>
#include <string>
#include "zdk/mutex.h"
#include "zdk/properties.h"
#include "zdk/ref_ptr.h"
#include "zdk/stream.h"
#include "zdk/zobject_impl.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"

/**
 * Generic, streamable properties implementation
 */
class ZDK_LOCAL PropertiesImpl : public ZObjectImpl<Properties>
{
public:

BEGIN_INTERFACE_MAP(PropertiesImpl)
    INTERFACE_ENTRY(PropertiesImpl)
    INTERFACE_ENTRY(Properties)
END_INTERFACE_MAP()

    PropertiesImpl();

    virtual ~PropertiesImpl() throw();

    virtual word_t get_word(const char*, word_t);

    virtual void set_word(const char*, word_t);

    virtual double get_double(const char*, double);

    virtual void set_double(const char*, double);

    virtual const char* get_string(const char*, const char*);

    virtual void set_string(const char*, const char*);

    virtual ZObject* get_object(const char*) const;

    virtual void set_object(const char* name, ZObject*);

    virtual size_t write(OutputStream*) const;

    void merge(const PropertiesImpl&);

private:
    typedef ext::hash_map<std::string, word_t> WordMap;
    typedef ext::hash_map<std::string, double> DoubleMap;
    typedef ext::hash_map<std::string, std::string> StringMap;
    typedef ext::hash_map<std::string, RefPtr<ZObject> > ObjectMap;

    WordMap wordMap_;
    DoubleMap doubleMap_;
    StringMap stringMap_;
    ObjectMap objectMap_;

protected:
    mutable Mutex mutex_;
};

#endif // PROPERTIES_H__8D476F2F_5AD8_4A8B_A075_3BDA6D0FF48F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
