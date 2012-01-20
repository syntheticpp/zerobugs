#ifndef ERROR_H__83F467C4_50AD_446A_8F6E_0F2509585A72
#define ERROR_H__83F467C4_50AD_446A_8F6E_0F2509585A72
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

#include "zdk/export.h"
#include "zdk/stdexcept.h"
#include <boost/shared_ptr.hpp>
#include <libelf.h>
#include <libdwarf.h>
#include "interface.h"

// __PRETTY_FUNCTION__ is a gcc extension; __func__ is C99 standard
#if defined(__GNUC__)
 // do nothing
#elif !defined(__func__)
  #define __func__ ?func?
#endif

#if 0
#define THROW_ERROR(dbg, err, ...) do { \
    Error::Throw(__func__, (dbg),(err),##__VA_ARGS__); } while (false)
#else
#define THROW_ERROR(dbg, err) while (true) \
    Error::Throw(__func__, (dbg),(err),__FILE__,__LINE__)
#endif

namespace Dwarf
{
    /**
     * Wrap Dwarf_Error into a standard C++ exception
     */
    class ZDK_EXPORT Error : public std::exception
    {
    protected:
        Error(const char* func, Dwarf_Debug, Dwarf_Error);

        ~Error() throw ();

        const char* what() const throw();

    public:
        static void Throw(
            const char* func,
            Dwarf_Debug,
            Dwarf_Error,
            const char* file = NULL,
            size_t line = 0);

        static std::string Message(
            Dwarf_Debug,
            Dwarf_Error,
            const char* file = NULL,
            size_t line = 0);

    private:
        class Impl;

        boost::shared_ptr<Impl> impl_;
    };
}
#endif // ERROR_H__83F467C4_50AD_446A_8F6E_0F2509585A72
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
