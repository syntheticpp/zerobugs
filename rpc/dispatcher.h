#ifndef DISPATCHER_H__ED731B3D_3335_49B8_8186_CB51D18EF7C5
#define DISPATCHER_H__ED731B3D_3335_49B8_8186_CB51D18EF7C5

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "dharma/properties.h"
#include "generic/empty.h"
#include "zdk/ref_ptr.h"
#include "zdk/stdexcept.h"
#include "zdk/stream.h"
#include "rpc/msg.h"


namespace RPC
{
    struct ZDK_LOCAL ServerSide { };
    struct ZDK_LOCAL ClientSide { };

    template<typename T, typename S>
    struct ZDK_LOCAL MessageTraits
    {
        template<typename F> static void register_with_factory(F f)
        {
            f->register_interface(T::_uuid(), T::create_cli);
        }

    };
    template<typename T>
    struct ZDK_LOCAL MessageTraits<T, ServerSide>
    {
        template<typename F> static void register_with_factory(F f)
        {
         /* char uuid[37];
            uuid_to_string(T::_uuid(), uuid);

            std::clog << __func__ << ": " << uuid << std::endl; */

            f->register_interface(T::_uuid(), T::create_srv);
        }
    };


    CLASS Dispatcher : public InputStreamEvents, public PropertiesImpl
    {
        /*** InputStreamEvents interface ***/
        virtual void on_word(const char* name, word_t val);

        virtual void on_double(const char*, double);

        virtual void on_string(const char*, const char*);

        virtual size_t on_object_begin(
                InputStream*,
                const char* name,
                uuidref_t   uuid,
                size_t      size);

        virtual void on_object_end() { }
        virtual void on_opaque(const char*, uuidref_t, const uint8_t*, size_t);
        virtual void on_bytes(const char*, const void*, size_t);

    public:
        template<typename S>
        Dispatcher(ObjectFactory& f, OutputStream& out, S side)
            : factory_(f)
            , output_(out)
            , server_(boost::is_same<S, ServerSide>::value)
        {
            MessageTraits<Error, S>::register_with_factory(&f);
            MessageTraits<Exec, S>::register_with_factory(&f);
            MessageTraits<Kill, S>::register_with_factory(&f);
            MessageTraits<Ptrace, S>::register_with_factory(&f);
            MessageTraits<RemoteIO, S>::register_with_factory(&f);
            MessageTraits<SysInfo, S>::register_with_factory(&f);
            MessageTraits<Waitpid, S>::register_with_factory(&f);
        }

        ~Dispatcher() throw();

        bool is_server_side() const { return server_; }

        bool read_and_dispatch_msg(InputStream&);

        RefPtr<ZObject> response() const { return response_; }

    private:
        ObjectFactory&  factory_;
        RefPtr<Message> message_;
        OutputStream&   output_;
        RefPtr<ZObject> response_;  // most recent obj sent back by the other side
        bool            server_;    // running on the server side?
    };
}
#endif // DISPATCHER_H__ED731B3D_3335_49B8_8186_CB51D18EF7C5
