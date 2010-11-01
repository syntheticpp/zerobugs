#ifndef PERSISTENT_H__0BD5C626_998F_4CFA_9F32_9499D0AE8440
#define PERSISTENT_H__0BD5C626_998F_4CFA_9F32_9499D0AE8440
//
// $Id: persistent.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include "zdk/export.h"
#include "zdk/ref_counted.h"
#include "zdk/stream.h"
#include <string>


class ZDK_LOCAL Persistent : public Streamable, public InputStreamEvents
{
public:
    const char* name() const { return name_.c_str(); }

protected:
    explicit Persistent(const char* name);

    virtual ~Persistent() throw();

BEGIN_INTERFACE_MAP(Persistent)
    INTERFACE_ENTRY(Streamable)
    INTERFACE_ENTRY(InputStreamEvents)
END_INTERFACE_MAP()

    void on_word(const char*, word_t);

    void on_double(const char*, double);

    void on_string(const char*, const char*);

    size_t on_object_begin(InputStream*, const char*, uuidref_t, size_t);

    void on_object_end() { }

    void on_opaque(const char*, uuidref_t, const uint8_t*, size_t);
    void on_bytes(const char* name, const void*, size_t);

private:
    void error(const char*, const char*);

    std::string name_;
};

#endif // PERSISTENT_H__0BD5C626_998F_4CFA_9F32_9499D0AE8440
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
