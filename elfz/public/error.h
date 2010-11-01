#ifndef ERROR_H__263AE63F_2CDA_49F2_807C_30A94D1E8350
#define ERROR_H__263AE63F_2CDA_49F2_807C_30A94D1E8350
//
// $Id: error.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#if (__GNUC__ >= 4) && !defined(__INTEL_COMPILER)
 #pragma GCC visibility push(default)
#endif
#include <exception>
#include <boost/shared_ptr.hpp>


namespace ELF
{
    /**
     * Wrapper class for libelf errors, translates
     * error codes into human-readable messages.
     *
     * @note for now, this is NOT a base class for
     * exceptions that libelfz (the C++ lib) may throw;
     * rather, the <stdexcept> classes are used.
     */
    class Error : public std::exception
    {
    public:
        /* ctor with perror()-like behavior:
         * the user supplied string will be followed
         * by a colon, a blank, and dwarf_errmsg()
         * when what() is called.
         */
        explicit Error(const char*);
        ~Error() throw ();

        const char* what() const throw();

    private:
        class Impl;

        boost::shared_ptr<Impl> impl_;
    };
}

#if (__GNUC__ >= 4) && !defined(__INTEL_COMPILER)
 #pragma GCC visibility pop
#endif

#endif // ERROR_H__263AE63F_2CDA_49F2_807C_30A94D1E8350
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
