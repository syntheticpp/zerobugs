// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: proxy-target.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <vector>
#include "dharma/canonical_path.h"
#include "dharma/cstream.h"
#include "dharma/environ.h"
#include "dharma/exec_arg.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"
#include "dharma/virtual_dso.h"
#include "engine/debugger_base.h"
#include "engine/process.h"
#include "engine/ptrace.h"
#include "engine/thread.h"
#include "symbolz/private/symbol_map_impl.h"
#include "target/memory_access.h"
#include "rpc/rpc.h"
#include "rpc/remote_ifstream.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/symbol_table.h"
#include "proxy-target.h"


using namespace std;
using namespace eventlog;


/**
 * Implements the Xtrace::Services interface by proxy-ing
 * waitpid / exec / ptrace on to the remote zserver.
 */
CLASS RemoteServices : public RefCountedImpl<XTrace::Services>
{
    RefPtr<XTrace::Services>    srv_;
    ObjectFactory&              f_;
    RPC::Stream&                stream_;

    int kill(pid_t, int, bool all);

public:
    RemoteServices(

        Debugger&       /* dbg */,
        ObjectFactory&  f,
        RPC::Stream&    s)

        : srv_(XTrace::services())
        , f_(f)
        , stream_(s)
    { }
    ~RemoteServices() throw()
    { }

    void restore()
    {
    #if DEBUG
        clog << __PRETTY_FUNCTION__ << endl;
    #endif
        XTrace::set_services(srv_);
    }

    pid_t waitpid(pid_t pid, int* status, int options);

    long ptrace(__ptrace_request, pid_t, XTrace::Arg, XTrace::Arg);

    void exec(char* const argv[], const char* const* env)
    {
        if (!srv_)
            throw runtime_error("RemoteServices::exec() null XTrace::Services pointer");
        srv_->exec(argv, env);
    }
    int kill(pid_t pid, int nsig)
    {
        return kill(pid, nsig, true);
    }
    int kill_thread(pid_t pid, int nsig)
    {
        return kill(pid, nsig, false);
    }
};


pid_t RemoteServices::waitpid(pid_t pid, int* status, int options)
{
    RefPtr<RPC::Waitpid> msg(new RPC::Waitpid(pid, options));

    try
    {
        if (RPC::send(f_, __func__, stream_, msg))
        {
            pid = RPC::value<RPC::wait_pid>(*msg);
            word_t stat = RPC::value<RPC::wait_stat>(*msg);

            if (status)
            {
                *status = stat;
            }
        }
        else
        {
            throw runtime_error(string(__func__) +  ": RPC error");
        }
    }
    catch (const SystemError& e)
    {
        errno = e.error();
        pid = -1;
    }
    return pid;
}


pid_t RemoteServices::kill(pid_t pid, int sig, bool all)
{
    RefPtr<RPC::Kill> msg(new RPC::Kill);

    RPC::value<RPC::kill_pid>(*msg) = pid;
    RPC::value<RPC::kill_sig>(*msg) = sig;
    RPC::value<RPC::kill_all>(*msg) = all;

    try
    {
        if (!RPC::send(f_, __func__, stream_, msg))
        {
            throw runtime_error(string(__func__) +  ": RPC error");
        }
    }
    catch (const SystemError& e)
    {
        errno = e.error();
        return -1;
    }
    return 0;
}


long RemoteServices::ptrace(__ptrace_request req,
                            pid_t pid,
                            XTrace::Arg addr,
                            XTrace::Arg data)
{
    RefPtr<RPC::Ptrace> msg(new RPC::Ptrace(req, pid, addr.i_, data.i_));

    switch (req)
    {
    case PTRACE_SETREGS:
        //Copies the general purpose registers, respectively, from location data
        //addr is ignored.
        msg->put(*reinterpret_cast<user_regs_struct*>(data.p_));
        break;
    case PTRACE_SETFPREGS:
        msg->put(*reinterpret_cast<user_fpregs_struct*>(data.p_));
        break;
#if HAVE_STRUCT_USER_FPXREGS_STRUCT
    case PTRACE_SETFPXREGS:
        msg->put(*reinterpret_cast<user_fpxregs_struct*>(data.p_));
        break;
#endif

    default:
        break;
    }

    long ret = -1;
    try
    {
        if (RPC::send(f_, __func__, stream_, msg))
        {
            ret = RPC::value<RPC::ptrace_ret>(*msg);

            switch (req)
            {
            case PTRACE_GETREGS:
                msg->get(*reinterpret_cast<user_regs_struct*>(data.p_));
                break;
            case PTRACE_GETFPREGS:
                msg->get(*reinterpret_cast<user_fpregs_struct*>(data.p_));
                break;

        #if HAVE_STRUCT_USER_FPXREGS_STRUCT
            case PTRACE_GETFPXREGS:
                msg->get(*reinterpret_cast<user_fpxregs_struct*>(data.p_));
                break;
        #endif

            case PTRACE_GETSIGINFO:
                // Retrieve  information about the signal that caused the stop.
                // Copies a siginfo_t structure from the child to location "data".
                // addr is ignored.
                msg->get(*reinterpret_cast<siginfo_t*>(data.p_));
                break;
            case PTRACE_GETEVENTMSG:
                // Retrieve  a  message (as an unsigned long)
                // placing it in the location data  (addr is ignored)
                msg->get(*reinterpret_cast<unsigned long*>(data.p_));
                break;

            default:
                break;
            }
        }
        else
        {
            throw runtime_error(string(__func__) +  ": RPC error");
        }
    }
    catch (const SystemError& e)
    {
        errno = e.error();
        assert(ret == -1);
    }
    return ret;
}




/**
 * The remote filesystem may be mounted on to the debugger's
 * workstation; a variable environment map hostnames to their
 * respective mountpoints.
 * E.g. ZERO_REMOTE_MAP="10.0.1.6:/mnt/limax;siegfried.zerobugs.org:/mnt/konrad;"
 */
static void read_host_map(map<string, string>& hostMap)
{
    string mapstr = env::get_string("ZERO_REMOTE_MAP");

    pair<size_t, size_t> key;
    pair<size_t, size_t> val;

    for (;; key.first = val.second + 1)
    {
        key.second = mapstr.find(':', key.first);
        if (key.second == string::npos)
        {
            break;
        }
        val.first = key.second + 1;
        val.second = mapstr.find(';', val.first);
        if (val.second == string::npos)
        {
            break;
        }

        hostMap[mapstr.substr(key.first, key.second - key.first)] =
                mapstr.substr(val.first, val.second - val.first);
    }
}


static void addr_to_id(const sockaddr_in& servAddr, string& id)
{
    ostringstream addr;
    addr << "remote://" << inet_ntoa(servAddr.sin_addr) << ":" << htons(servAddr.sin_port);
    id = addr.str();
}


static void
connect(const string& hostname,
        sockaddr_in& servAddr,
        auto_fd& sock,
        string& mountPoint)
{
    map<string, string> hostMap;
    read_host_map(hostMap);

    mountPoint = hostMap[hostname];

    if (inet_aton(hostname.c_str(), &servAddr.sin_addr) == 0) // not an addr?
    {
        if (const hostent* entry = gethostbyname(hostname.c_str()))
        {
            memcpy(&servAddr.sin_addr, entry->h_addr, sizeof(in_addr));
            if (mountPoint.empty())
            {
                mountPoint = hostMap[inet_ntoa(servAddr.sin_addr)];
            }
        }
        else
        {
            throw runtime_error(hstrerror(h_errno));
        }
    }
    if (::connect(sock.get(), (const sockaddr*)&servAddr, sizeof (servAddr)) < 0)
    {
        throw SystemError("connect");
    }
    if (!mountPoint.empty())
    {
        mountPoint = CanonicalPath(mountPoint);
    }
}


/**
 * Connect to the remote server.
 * @return the path to the remote executable.
 */
static string
connect(const string& s, auto_fd& sock, string& mountPoint, string& id)
{
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof (servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(RPC::port);

    string hostname, result;

    size_t hostBegin = s.find("://");
    if (hostBegin == s.npos)
    {
        hostBegin = 0;
    }
    else
    {
        hostBegin += 3;
    }
    size_t hostEnd = s.find('/', hostBegin);
    if (hostEnd == string::npos)
    {
        hostname = s.substr(hostBegin);
    }
    else
    {
        result = s.substr(hostEnd);
        hostname = s.substr(hostBegin, hostEnd);
    }
    hostEnd = hostname.find(':'); // look for port number
    if (hostEnd != string::npos)
    {
        const char* port = hostname.c_str() + hostEnd + 1;
        servAddr.sin_port = boost::lexical_cast<int>(port);
        servAddr.sin_port = htons(servAddr.sin_port);
        hostname.resize(hostEnd);
    }
    connect(hostname, servAddr, sock, mountPoint);
    addr_to_id(servAddr, id);
    return result;
} // connect


ProxyTarget::ProxyTarget(debugger_type& dbg, size_t size)
    : LinuxLiveTarget(dbg, size)
    , sock_(socket(AF_INET, SOCK_STREAM, 0))
    , stream_(sock_)
    , xtraceSrv_(new RemoteServices(dbg, factory(), stream_))
{
    if (!sock_.is_valid())
    {
        throw SystemError("socket");
    }
    XTrace::set_services(xtraceSrv_);
}


ProxyTarget::~ProxyTarget() throw()
{
    dbgout(0) << __func__ << endl;
    xtraceSrv_->restore();
}


ObjectFactory& ProxyTarget::factory() const
{
    return interface_cast<ObjectFactory&>(debugger());
}


void ProxyTarget::init(const char* param)
{
    Target::init(param);
    arg0_ = connect(param, sock_, mountPoint_, id_);
}


/**
 * Execute a command (with optional environment) and
 * return the main thread of the newly spawned process
 */
Thread* ProxyTarget::exec(const ExecArg& args, const char* const* env)
{
    assert(!arg0_.empty());

    ExecArg remoteArgs(args);

    remoteArgs.pop_front();
    remoteArgs.push_front(arg0_);
    const string& path = arg0_;
    dbgout(1) << __func__ << ": " << remoteArgs.command_line() << endl;

    RefPtr<RPC::Exec> msg = new RPC::Exec(remoteArgs, false, env);

    if (RPC::send(factory(), __func__, stream_, msg))
    {
        assert(msg);

        const pid_t pid = RPC::value<RPC::exec_pid>(*msg);
        dbgout(0) << __func__ << ": remote process pid=" << pid << endl;

        int status = 0;
        sys::waitpid(pid, &status, 0);

        init_process(pid, &args, ORIGIN_DEBUGGER, path.c_str());
        assert(process());

        init_symbols();

        long id = 0; // todo
        RefPtr<ThreadImpl> thread(new ThreadImpl(*this, id, pid));
        thread->set_status(status);

        add_thread(thread);
        debugger().add_target(this); // make sure this target is managed
        debugger().on_attach(*thread);

        return thread.detach();
    }
    else
    {
        throw runtime_error(string(__func__) +  ": RPC error");
    }
}


string ProxyTarget::get_system_release() const
{
    RefPtr<RPC::SysInfo> msg = new RPC::SysInfo();

    if (RPC::send(factory(), __func__, stream_, msg))
    {
        if (RPC::value<RPC::remote_byte_order>(*msg) != __BYTE_ORDER
         || RPC::value<RPC::remote_word_size>(*msg) != __WORDSIZE)
        {
            throw runtime_error("remote system not supported: "
                + RPC::value<RPC::remote_system>(*msg));
        }

        return RPC::value<RPC::remote_sysver>(*msg);
    }
    else
    {
        throw runtime_error(string(__func__) +  ": RPC error");
    }
}


void ProxyTarget::read_memory
(
    pid_t       pid,
    SegmentType seg,
    addr_t      addr,
    long*       buf,
    size_t      len,
    size_t*     nRead
) const
{
    Memory<>::read_using_ptrace(pid, seg, addr, buf, len, nRead);
}


VirtualDSO* ProxyTarget::read_virtual_dso() const
{
    VirtualDSO* result = 0;
    
    if (process()) try
    {
        addr_t addr = 0;
        size_t sz = 0;

        AuxVect v = aux_vect();

        if (get_sysinfo_ehdr(v, addr, sz))
        {
            vector<char> buf(sz);

            // read from /proc/PID/mem on the remote target
            ostringstream mem;
            mem << procfs_root() << process()->pid() << "/mem";

            auto_ptr<remote_ifstream> targetMemory(
                new remote_ifstream(factory(), stream_, mem.str().c_str()));

            targetMemory->seek(addr);
            targetMemory->read(&buf[0], buf.size());

            result = new VirtualDSO(buf, addr);
        }
    }
    catch (const exception& e)
    {
        dbgout(0) << __func__ << ": " << e.what() << endl;
    }
    return result;
}


const string& ProxyTarget::procfs_root() const
{
    if (procRoot_.empty())
    {
        procRoot_ = env::get_string("ZERO_PROCFS", "");
        if (procRoot_.empty())
        {
            dbgout(0) << __func__ << ": mount point=" << mountPoint_ << endl;
            if (mountPoint_.empty())
            {
                procRoot_ = LinuxLiveTarget::procfs_root();
            }
            else
            {
                procRoot_ = mountPoint_ + "/proc/";
            }
        }
        dbgout(0) << __func__ << ": " << procRoot_ << endl;
    }
    return procRoot_;
}


bool ProxyTarget::map_path(string& path) const
{
    if (mountPoint_.empty())
    {
        return false;
    }
    assert(path.empty() || path[0] == '/');
    //prevent prepending the mountPoint twice
    if (strncmp(path.c_str(), mountPoint_.c_str(), mountPoint_.size()) == 0)
    {
        return false;
    }
    path = mountPoint_ + path;
    dbgout(0) << __func__ << ": " << path << endl;
    return true;
}


auto_ptr<istream> ProxyTarget::get_ifstream(const char* filename) const
{
    return auto_ptr<istream>(new remote_ifstream(factory(), stream_, filename));
}


RefPtr<SharedString> ProxyTarget::process_name(pid_t pid) const
{
    if (pid == 0)
    {
        return Target::process_name(pid);
    }
    ostringstream exe;
    exe << "/proc/" << pid << "/exe";
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(exe.str().c_str());
    const size_t len = exe.str().size() + 1;
    RefPtr<RPC::RemoteIO> msg =
        new RPC::RemoteIO(RPC::RIO_READ_LINK, bytes, len);
    if (!RPC::send(factory(), __func__, stream_, msg))
    {
        throw runtime_error(string(__func__) +  ": RPC error");
    }
    const vector<uint8_t>& buf = RPC::value<RPC::rio_data>(*msg);
    assert(!buf.empty());
    return shared_string(reinterpret_cast<const char*>(&buf[0]));
}
