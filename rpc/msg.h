#ifndef MSG_H__530DC59F_CD30_4015_A674_58B04FDD0523
#define MSG_H__530DC59F_CD30_4015_A674_58B04FDD0523

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <string>
#include <vector>
#include <sys/ptrace.h>
#include "zdk/stream.h"
#include "zdk/zobject_impl.h"
#include "dharma/exec_arg.h"
#include "dharma/system_error.h"
#include "rpc/attr.h"
#include "rpc/remote_io.h"


namespace RPC
{
    enum
    {
        port = 30000, // default TCP port
    };


    /**
     * Base RPC message
     */
    struct Message
        : public Streamable
        , public InputStreamEvents
        , public ZObjectImpl<>
    {
        DECLARE_UUID("282aa538-52ad-438e-877c-4712148bb8c9")

        ~Message() throw();

        virtual bool handle(InputStream&, OutputStream&) = 0;

        /*** implements the InputStreamEvents interface ***/
        void on_word(const char* name, word_t val);

        void on_double(const char*, double);

        void on_string(const char*, const char*);

        size_t on_object_begin(InputStream*, const char*, uuidref_t, size_t);
        void on_object_end();

        void on_opaque(const char*, uuidref_t, const uint8_t*, size_t);
        void on_bytes(const char*, const void*, size_t);
    };


    /**
     * partial message implementation
     */
    template<typename T>
    struct MessageImpl : public Message
    {
    BEGIN_INTERFACE_MAP(T)
        INTERFACE_ENTRY(T)
        INTERFACE_ENTRY(Message)
        INTERFACE_ENTRY(ZObject)
    END_INTERFACE_MAP()

        ~MessageImpl() throw() { }

        uuidref_t uuid() const { return T::_uuid(); }

        bool handle(InputStream& in, OutputStream& out)
        {
            assert (dispatch_);
            return (static_cast<T*>(this)->*dispatch_)(in, out);
        }
        static ZObject* create_srv(ObjectFactory* fact, const char* name)
        {
            assert(fact);
            T* msg = new T;
            msg->dispatch_ = &T::dispatch;
            msg->name_ = name;
            return msg;
        }
        static ZObject* create_cli(ObjectFactory*, const char* name)
        {
            T* msg = new T;
            msg->name_ = name;
            return msg;
        }

        const char* name() const { return name_.c_str(); }

    protected:
        MessageImpl() : dispatch_(NULL) { }
        size_t write(OutputStream*) const { return 0; }

        bool (T::*dispatch_)(InputStream&, OutputStream&);

    private:
        std::string name_;
    };


    /**
     * negotiate communication details w/ the remote platform
     *
     * @note WORK IN PROGRESS
     */
    CLASS SysInfo : public Attr<remote_word_size, word_t,
                           Attr<remote_byte_order, word_t,
                           Attr<remote_system, std::string,
                           Attr<remote_sysver, std::string,
                           MessageImpl<SysInfo> > > > >
    {
    public:
        DECLARE_UUID("a3532e85-9b45-461f-8852-59f2e44c67f1")

        ~SysInfo() throw();
        SysInfo() { };

        bool dispatch(InputStream&, OutputStream&);
    };

    /**
     * A class for transporting exceptions between
     * the server and the client side.
     */
    CLASS Error : public Attr<err_errno, word_t,
                         Attr<err_what, std::string,
                         MessageImpl<Error> > >
    {
    public:
        DECLARE_UUID("3cf22043-dd48-4064-9176-c175ea3c1ba8")

        ~Error() throw();
        Error() { dispatch_ = &Error::dispatch; }
        explicit Error(const std::exception&);
        explicit Error(const SystemError&);

        bool dispatch(InputStream&, OutputStream&);
    };


    /**
     * Instruct the server to execute a target program.
     *
     * @todo pass environment to debug target
     */
    CLASS Exec : public Attr<exec_cmd, std::string,
                        Attr<exec_pid, word_t,
                        MessageImpl<Exec> > >
    {
    public:
        DECLARE_UUID("ad0524ea-5db4-46a2-ae8f-2f0a346d2be9")

        ~Exec() throw();

        /// client-side ctor
        Exec(const ExecArg&, bool, const char* const*);
        Exec() { };

        bool dispatch(InputStream&, OutputStream&);
    };


    /**
     * Call waitpid() on remote server
     */
    CLASS Waitpid : public Attr<wait_pid, word_t,
                           Attr<wait_opt, word_t,
                           Attr<wait_stat, word_t,
                           MessageImpl<Waitpid> > > >
    {
    public:
        DECLARE_UUID("6cb73425-fe74-4c59-8f89-f1f0e22c2906")

        Waitpid() { };
        Waitpid(pid_t pid, int opt);
        ~Waitpid() throw();

        bool dispatch(InputStream&, OutputStream&);
    };



    /**
     * Call ptrace on remote server.
     *
     * The "bits" attribute is for marshalling bufferfuls of data
     * for requests that need it, such as PTRACE_SETREGS, etc.
     */
    CLASS Ptrace :
        public Attr<ptrace_req,  word_t,
               Attr<ptrace_pid,  word_t,
               Attr<ptrace_addr, word_t,
               Attr<ptrace_data, word_t,
               Attr<ptrace_ret,  word_t,
               Attr<ptrace_bits, std::vector<uint8_t>,
               MessageImpl<Ptrace> > > > > > >
    {
    public:
        DECLARE_UUID("641a0e3d-2938-479e-98b1-41a1fd214322")

        Ptrace() {}
        Ptrace(__ptrace_request, word_t, word_t, word_t);
        ~Ptrace() throw();

        bool dispatch(InputStream&, OutputStream&);
    };


    CLASS Kill : public Attr<kill_pid, word_t,
                        Attr<kill_sig, word_t,
                        Attr<kill_all, word_t,
                        MessageImpl<Kill> > > >
    {
    public:
        DECLARE_UUID("273592ae-1798-479e-ad9a-9c222db30db3")

        Kill() { };
        Kill(pid_t pid, int opt);
        ~Kill() throw();

        bool dispatch(InputStream&, OutputStream&);
    };


    /**
     *  This message is sent to the server to perform an I/O operation.
     *  The rio_op attribute specifies the operation, which takes
     *  an IOCommand enumerated value.
     *  For the RIO_OPEN operation, the file descriptor (if successful)
     *  is returned in the rio_file attribute. rio_file is expected to be
     *  sent in with subsequent RIO_READ, RIO_SEEK, etc messages.
     *  The rio_data vector is expected to hold thefilename for RIO_OPEN
     *  and RIO_READ_LINK, and upon return holds the data read for RIO_READ.
     */
    CLASS RemoteIO : public Attr<rio_op,   word_t,
                            Attr<rio_file, word_t,
                            Attr<rio_data, std::vector<uint8_t>,
                            MessageImpl<RemoteIO> > > >
    {
    public:
        DECLARE_UUID("130783a2-ded8-45c1-aab1-2904a5b23229")

        RemoteIO() { }
        RemoteIO(IOCommand, const uint8_t* data, size_t);
        explicit RemoteIO(const char* filename)
        {
            assert(filename);
            RPC::value<rio_op>(*this) = RPC::RIO_OPEN;
            RPC::value<rio_data>(*this).assign(filename, filename + strlen(filename) + 1);
        }
        ~RemoteIO() throw();

        bool dispatch(InputStream&, OutputStream&);
    };

} // namespace RPC
#endif // MSG_H__530DC59F_CD30_4015_A674_58B04FDD0523
