#ifndef FACTORY_H__B82A6EB1_6730_4BA0_8186_F77CB0BFDF72
#define FACTORY_H__B82A6EB1_6730_4BA0_8186_F77CB0BFDF72
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: factory.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "zdk/stream.h"
#include "zdk/zobject_impl.h"


/**
 * Instantiates objects on the server side based on parameters
 * deserialized from client requests.
 */
CLASS Factory : public ZObjectImpl<ObjectFactory>
{
    typedef ext::hash_map<ZDK_UUID, ObjectFactory::CreatorFunc> FactoryMap;

public:
BEGIN_INTERFACE_MAP(Factory)
    INTERFACE_ENTRY(ObjectFactory)
END_INTERFACE_MAP()

    Factory() { }
    ~Factory() throw() { }

    void register_interface(uuidref_t, ObjectFactory::CreatorFunc);
    bool unregister_interface(uuidref_t);
    ZObject* create_object(uuidref_t, const char* name);

private:
    FactoryMap factoryMap_;
};
#endif // FACTORY_H__B82A6EB1_6730_4BA0_8186_F77CB0BFDF72
