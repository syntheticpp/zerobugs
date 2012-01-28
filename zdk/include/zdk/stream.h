#ifndef STREAM_H__8FEA9400_8835_40AA_B68D_AF9FBE93339F
#define STREAM_H__8FEA9400_8835_40AA_B68D_AF9FBE93339F
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

#include "zdk/platform.h"
#include "zdk/zobject.h"


using Platform::word_t;

struct RefCounted;
struct Streamable;
struct InputStreamEvents;


/**
 * An interface for writing name-value pairs.
 */
DECLARE_ZDK_INTERFACE_(OutputStream, struct Unknown)
{
    DECLARE_UUID("37704090-7ad6-4447-bedf-95011a5717ef")

    virtual size_t write_word(const char* name, word_t val) = 0;

    virtual size_t write_double(const char* name, double val) = 0;

    virtual size_t write_string(const char*, const char*) = 0;

    virtual size_t write_object(const char*, const Streamable*) = 0;

    virtual size_t write_opaque(const uint8_t*, size_t) = 0;

    virtual size_t write_bytes(const char*, const void*, size_t) = 0;
};


/**
 * Reads name-value pairs, calling back into the supplied
 * InputStreamEvents interface.
 */
DECLARE_ZDK_INTERFACE_(InputStream, struct Unknown)
{
    DECLARE_UUID("de480fff-387c-4d44-8d18-9776c018b4bb")

    /**
     * Read the next name-value pair, return the number
     * of bytes read, zero if at end of stream.
     */
    virtual size_t read(InputStreamEvents*) = 0;
};


/**
 * Receives name-value pair notifications from the InputStream.
 */
DECLARE_ZDK_INTERFACE_(InputStreamEvents, struct Unknown)
{
    DECLARE_UUID("cf20beb0-51ab-497d-ba07-458b8c6aa6d6")

    virtual void on_word(const char* name, word_t val) = 0;

    virtual void on_double(const char*, double) = 0;

    virtual void on_string(const char*, const char*) = 0;

    virtual size_t on_object_begin(
            InputStream*,
            const char* name,
            uuidref_t   uuid,
            size_t      size) = 0;

    virtual void on_opaque(const char* name,
                           uuidref_t uuid,
                           const uint8_t* buf,
                           size_t buflen) = 0;

    virtual void on_object_end() = 0;

    virtual void on_bytes( const char* name,
                           const void* buf,
                           size_t buflen) = 0;
};


DECLARE_ZDK_INTERFACE_(InputStreamEvents64, InputStreamEvents)
{
    DECLARE_UUID("b07936bc-87c8-485b-acb5-773eb8c366c4")

    virtual void on_word64(const char* name, int64_t val) = 0;
};


DECLARE_ZDK_INTERFACE_(Streamable, struct Unknown)
{
    DECLARE_UUID("053088e2-6599-473a-a5ad-885bb89020ff")

    virtual uuidref_t uuid() const = 0;

    /**
     * @return the number of bytes written
     */
    virtual size_t write(OutputStream*) const = 0;
};


DECLARE_ZDK_INTERFACE_(ObjectFactory, ZObject)
{
    DECLARE_UUID("a3b43412-da37-4b6b-a1a2-d469bb1c4f8f")

    typedef ZObject* (*CreatorFunc)(ObjectFactory*, const char* name);

    virtual void register_interface(uuidref_t, CreatorFunc) = 0;

    virtual bool unregister_interface(uuidref_t) = 0;

    virtual ZObject* create_object(uuidref_t, const char* name) = 0;
};
#endif // STREAM_H__8FEA9400_8835_40AA_B68D_AF9FBE93339F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
