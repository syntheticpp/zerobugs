#ifndef OBJECT_MANAGER_H__65C4CA56_C697_4B84_B1D6_A731BC73A2AF
#define OBJECT_MANAGER_H__65C4CA56_C697_4B84_B1D6_A731BC73A2AF
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
// Classes for debugging leaked objects.
//   Usage:
//   step 1) derive your target class, say A, from CountedInstance<A>
//    (using Coplien pattern)
//
//   step 2) insert ObjectManager::Instace().print() in the program.
//
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include "zdk/unknown2.h"
#include "generic/singleton.h"


/**
 * A singleton that keeps count of instances of other objects
 * -- for debugging purposes (leak tracking)
 */
class ObjectManagerImpl
{
public:
    template<typename T> void add(const T* object)
    {
        std::pair<size_t, size_t>& p = instanceMap_[T::_type()];
        ++p.first;

        p.second += object->_size();
    }

    template<typename T> void remove(const T* object)
    {
        assert(object);

        instance_map_type::iterator i = instanceMap_.find(T::_type());
        remove(i, object->_size());
    }

    std::ostream& print(std::ostream&, const char*) const;

protected:
    ObjectManagerImpl();

    virtual ~ObjectManagerImpl();

    ObjectManagerImpl(const ObjectManagerImpl&);
    ObjectManagerImpl& operator=(const ObjectManagerImpl&);

private:
    // map the type names to instance count -- a hash map might be
    // more efficient, but this is for debug, so performance should
    // not matter that much
    typedef std::map<
        std::string, std::pair<size_t, size_t> > instance_map_type;

    void remove(instance_map_type::iterator, size_t);

    instance_map_type instanceMap_;
};


class ObjectManagerFactory;
typedef Singleton<ObjectManagerImpl, ObjectManagerFactory> ObjectManager;

class ObjectManagerFactory
{
    static ObjectManager* instance_;

public:
    typedef ObjectManagerFactory type;

    static ObjectManager& instance(void*)
    {
        if (instance_ == NULL)
        {
            instance_ = new ObjectManager();
        }
        return *instance_;
    }
};


/**
 * A base template class -- to be used with the Coplien
 * pattern for objects we want to instance-count.
 */
template<typename T> struct ZDK_VTABLE CountedInstance
{
#ifdef DEBUG_OBJECT_LEAKS
    CountedInstance()
    {
        ObjectManager::instance().add(static_cast<T*>(this));
    }

    CountedInstance(const CountedInstance&)
    {
        ObjectManager::instance().add(static_cast<T*>(this));
    }

    /**
     * @note the destructor promises not to throw but both
     * instance() and remove() can throw. However. I expected
     * to have the singleton instance created already -- from
     * the ctor of this object, or another ctor. Similarly, I
     * expect to have a map entry for this object type already.
     * So really, if the line below does result in more than
     * a search and decrement, things are screwed up badly. Abort!
     */
    virtual ~CountedInstance()
    {
        ObjectManager::instance().remove(static_cast<T*>(this));
    }

    static size_t _size() { return sizeof(T); }

#else
    virtual ~CountedInstance() {}

#endif // DEBUG_OBJECTS
};

#endif // OBJECT_MANAGER_H__65C4CA56_C697_4B84_B1D6_A731BC73A2AF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
