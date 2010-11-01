#ifndef DUMP_SETTINGS_H__2D6E1909_4BEB_4E18_948A_B85F6110CF1E
#define DUMP_SETTINGS_H__2D6E1909_4BEB_4E18_948A_B85F6110CF1E
//
// $Id: dump_settings.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/python.hpp>
#include <vector>
#include "zdk/stream.h"
#include "zdk/zobject_impl.h"


class ZDK_LOCAL DumpSettings : public ZObjectImpl<InputStreamEvents>
{
BEGIN_INTERFACE_MAP(DumpSettings)
    INTERFACE_ENTRY(InputStreamEvents)
END_INTERFACE_MAP()

public:
    DumpSettings();
    virtual ~DumpSettings() throw();

    virtual void on_word(const char*, word_t);

    virtual void on_double(const char*, double);

    virtual void on_string(const char*, const char*);

    virtual size_t on_object_begin(
            InputStream*    stream,
            const char*     name,
            uuidref_t       uuid,
            size_t          size);

    virtual void on_object_end() { }

    virtual void on_opaque(const char*, uuidref_t, const uint8_t*, size_t);
    virtual void on_bytes(const char*, const void*, size_t);

    const boost::python::list& list() const
    {
        return list_;
    }

private:
    boost::python::list list_;
    std::vector<boost::python::list> stack_;
};

#endif // DUMP_SETTINGS_H__2D6E1909_4BEB_4E18_948A_B85F6110CF1E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
