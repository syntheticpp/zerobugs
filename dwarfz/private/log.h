#ifndef LOG_H__EC55BE0B_3AB7_4013_A161_0CFD126C20C5
#define LOG_H__EC55BE0B_3AB7_4013_A161_0CFD126C20C5
//
// $Id: log.h 715 2010-10-17 21:43:59Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
//
// TODO: replace this with zdk/eventlog
//
namespace Dwarf
{
    /**
     * @return verbosity value, as set by the ZERO_DEBUG_DWARF
     * environment variable.
     */
    int debug_verbosity();

    enum LogLevel
    {
        info = 1,
        debug,
        warn,
        error,
    };

    template<LogLevel> struct LogLevelTraits
    {
        static const char* prefix() { return "info"; }
    };
    template<> struct LogLevelTraits<warn>
    {
        static const char* prefix() { return "warn"; }
    };
    template<> struct LogLevelTraits<error>
    {
        static const char* prefix() { return "error"; }
    };


    template<LogLevel L> struct log
    {
        log()
        {
            std::clog << "dwarfz-" << LogLevelTraits<L>::prefix() << ": ";
        }
        template<typename T> log& operator<<(T value)
        {
            std::clog << value;
            return *this;
        }
    };

    template<> class log<debug>
    {
        int verbosity_;

    public:
        explicit log(int verbosity = 0) : verbosity_(verbosity)
        { }

        template<typename T> log& operator<<(T value)
        {
        #ifdef DEBUG
            if (verbosity_ < debug_verbosity())
            {
                std::clog << value;
            }
        #endif
            return *this;
        }
    };

} // namespace Dwarf


#endif // LOG_H__EC55BE0B_3AB7_4013_A161_0CFD126C20C5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
