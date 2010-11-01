#ifndef FACTORY_H__ADD0D3B1_DB5A_4DDE_AFCA_4F6860E337A9
#define FACTORY_H__ADD0D3B1_DB5A_4DDE_AFCA_4F6860E337A9
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

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <libdwarf.h>
#include "zdk/mutex.h"
#include "dwarfz/public/interface.h"


namespace Dwarf
{
    class Die; // forward


    class Factory : boost::noncopyable
    {
        typedef boost::shared_ptr<Die>
            Creator(Dwarf_Debug, Dwarf_Die);

        typedef std::map<Dwarf_Half, Creator*> CreatorMap;

    public:
        static Factory& instance();

        bool register_creator(Dwarf_Half, Creator*);

        /**
         * If OWN is true, the die is deallocated even when
         * there's no creator function registered for it's tag;
         * if TAG is not null, will contain tag upon return.
         */
        boost::shared_ptr<Die> create(  Dwarf_Debug,
                                        Dwarf_Die,
                                        bool own = true,
                                        Dwarf_Half* tag = 0) const;

        boost::shared_ptr<Die> create(  Dwarf_Debug,
                                        Dwarf_Die,
                                        Dwarf_Half tag,
                                        bool own) const;

    private:
        Factory();
        ~Factory();

        CreatorMap creatorMap_;
        mutable Mutex mutex_;
    };
}

#endif // FACTORY_H__ADD0D3B1_DB5A_4DDE_AFCA_4F6860E337A9
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
