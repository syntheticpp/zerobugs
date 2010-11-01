#ifndef OBJECT_FACTORY_H__21E25B3A_891A_425B_8EF5_B5FBB08EFD71
#define OBJECT_FACTORY_H__21E25B3A_891A_425B_8EF5_B5FBB08EFD71

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/stream.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "dharma/settings.h"

struct Debugger;

typedef ext::hash_map<ZDK_UUID, ObjectFactory::CreatorFunc> FactoryMap;



class ZDK_LOCAL ObjectFactoryImpl : public ZObjectImpl<ObjectFactory>
{
public:
BEGIN_INTERFACE_MAP(ObjectFactoryImpl)
    INTERFACE_ENTRY(ObjectFactory)
    INTERFACE_ENTRY_AGGREGATE(debugger_)
END_INTERFACE_MAP()

    explicit ObjectFactoryImpl(Debugger* d);
    ~ObjectFactoryImpl() throw() { }

    void register_interface(uuidref_t, ObjectFactory::CreatorFunc);

    bool unregister_interface(uuidref_t);

    ZObject* create_object(uuidref_t, const char* name);

private:
    FactoryMap factoryMap_;
    Debugger* debugger_;
};

////////////////////////////////////////////////////////////////
static ZObject* create_settings(ObjectFactory* fact, const char*)
{
    return new Settings(fact);
}

////////////////////////////////////////////////////////////////
ObjectFactoryImpl::ObjectFactoryImpl(Debugger* d) : debugger_(d)
{
    register_interface(Settings::_uuid(), create_settings);
}

////////////////////////////////////////////////////////////////
void
ObjectFactoryImpl::register_interface(uuidref_t iid, CreatorFunc fn)
{
    assert(iid);
    factoryMap_.insert(std::make_pair(*iid, fn));
}


////////////////////////////////////////////////////////////////
bool ObjectFactoryImpl::unregister_interface(uuidref_t iid)
{
    assert(iid);
    FactoryMap::iterator i = factoryMap_.find(*iid);
    if (i != factoryMap_.end())
    {
        factoryMap_.erase(i);
        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////
ZObject*
ObjectFactoryImpl::create_object(uuidref_t iid, const char* name)
{
    assert(iid);
    FactoryMap::const_iterator i = factoryMap_.find(*iid);
    if (i != factoryMap_.end())
    {
        return (*i->second)(this, name);
    }
    return NULL;
}

#endif // OBJECT_FACTORY_H__21E25B3A_891A_425B_8EF5_B5FBB08EFD71
