#ifndef SETTINGS_H__07367374_99D3_4DC1_8CD4_FAF91176DFEE
#define SETTINGS_H__07367374_99D3_4DC1_8CD4_FAF91176DFEE
//
// $Id: settings.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <vector>
#include "dharma/properties.h"
#include "zdk/weak_ptr.h"



class ZDK_LOCAL Settings
    : public PropertiesImpl
    , public InputStreamEvents
    , public Streamable
{
public:
    DECLARE_UUID("aae5ebff-8449-41e2-8a0c-b28edebbf997")

BEGIN_INTERFACE_MAP(Settings)
    INTERFACE_ENTRY(Settings)
    INTERFACE_ENTRY(InputStreamEvents)
    INTERFACE_ENTRY(Streamable)
    INTERFACE_ENTRY_INHERIT(PropertiesImpl)
END_INTERFACE_MAP()

    explicit Settings(WeakPtr<ObjectFactory>);

    virtual ~Settings() throw();

    void erase_opaque_objects() { opaqueList_.clear(); }

    virtual void set_object(const char* name, ZObject*);

    virtual size_t write(OutputStream*) const;

    /***  Streamable ***/
    virtual uuidref_t uuid() const { return _uuid(); }

    /*** InputStreamEvents ***/
    virtual void on_word(const char*, word_t);

    virtual void on_double(const char*, double);

    virtual void on_string(const char*, const char*);

    virtual size_t on_object_begin(
            InputStream*    stream,
            const char*     name,
            uuidref_t       uuid,
            size_t          size);

    virtual void on_object_end() { } // do nothing

    virtual void on_opaque(const char*, uuidref_t, const uint8_t*, size_t);

    virtual void on_bytes( const char* name,
                           const void* buf,
                           size_t buflen) { }

private:
    typedef std::vector<uint8_t> Opaque;
    typedef std::vector<Opaque> OpaqueList;

    WeakPtr<ObjectFactory> factory_;
    OpaqueList opaqueList_;
};

#endif // SETTINGS_H__07367374_99D3_4DC1_8CD4_FAF91176DFEE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
