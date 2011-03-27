#ifndef TARGET_FACTORY_H__C0C44E52_3838_45B8_AE48_49F4D97DB84F
#define TARGET_FACTORY_H__C0C44E52_3838_45B8_AE48_49F4D97DB84F
//
// $Id: target_factory.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/ref_ptr.h"
#include "generic/singleton.h"
#include "debuggerfwd.h"
#include <map>
#include <string>

struct Target;


class TargetFactory
{
public:
    enum OSID
    {
        Linux,
        FreeBSD,
    #ifdef __linux__
        NativeOS = Linux
    #elif defined(__FreeBSD__)
        NativeOS = FreeBSD
    #endif
    };

    struct Key
    {
        OSID        osid_;
        unsigned    wordsize_;
        bool        live_;
        std::string subsystem_;
    };
    typedef RefPtr<Target> (*FuncPtr)(debugger_type&);
    typedef std::map<Key, FuncPtr> map_type;


    RefPtr<Target> create_target(OSID,
                         unsigned wordsize,
                                    // currently 32 or 64
                         bool live, // process or corefile?
                         const std::string& subsystem,
                         debugger_type&);

    bool register_target(OSID,
                         unsigned wordsize,
                         bool live,
                         const std::string&,
                         FuncPtr,
                         bool override = false);

private:
    map_type map_;
};

bool operator<(const TargetFactory::Key& lhs,
               const TargetFactory::Key& rhs);

typedef Singleton<TargetFactory> TheTargetFactory;

#endif // TARGET_FACTORY_H__C0C44E52_3838_45B8_AE48_49F4D97DB84F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
