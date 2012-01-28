#ifndef EVENTLOG_H__5E85E497_C4D7_474D_8A58_0F7320BC43BD
#define EVENTLOG_H__5E85E497_C4D7_474D_8A58_0F7320BC43BD
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

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <boost/utility.hpp>
#include "generic/lock.h"
#include "zdk/export.h"


namespace eventlog
{
    template<typename CharType>
    struct ZDK_LOCAL DefaultWriter { };


    template<> struct ZDK_LOCAL DefaultWriter<char>
    {
        static void flush(const std::string& logStreamText)
        {
            std::clog << logStreamText;
        }
    };
    template<> struct ZDK_LOCAL DefaultWriter<wchar_t>
    {
        static void flush(const std::wstring& logStreamText)
        {
            std::wclog << logStreamText;
        }
    };



    template<typename T,
             typename CharType = char,
             typename W = DefaultWriter<CharType>
            >
    struct ZDK_LOCAL Channel : public W
    {
        typedef T type;
        typedef CharType char_type;
        typedef std::basic_ostringstream<char_type> Buffer;

        static std::auto_ptr<Buffer> buf_;

        template<typename U>  static void prefix(U&) { }
        template<typename U>  static void postfix(U&) { }

        static void enter() { }
        static void leave() { }

        int level() const { return 1; }

        Buffer& buffer()
        {
            Lock<T> lock(static_cast<T&>(*this));
            if (!buf_.get())
            {
                buf_.reset(new Buffer);
                static_cast<T&>(*this).prefix(*buf_);
            }
            return *buf_;
        }
        bool flush()
        {
            Lock<T> lock(static_cast<T&>(*this));
            if (buf_.get())
            {
                static_cast<T&>(*this).postfix(*buf_);
                W::flush(buf_->str());
                buf_.reset();
                return true;
            }
            return false;
        }
    };

    template<typename T, typename C, typename W>
    std::auto_ptr<std::basic_ostringstream<C> > Channel<T, C, W>::buf_;


    struct ZDK_LOCAL DefaultChannel : public Channel<DefaultChannel>
    {
    };



    template<typename T = DefaultChannel>
    class ZDK_LOCAL Stream
    {
        typedef typename T::char_type char_type;

        typedef std::basic_ostringstream<char_type> Buffer;
        typedef std::basic_ostream<char_type> OutputStream;

        int level_;
        T channel_;

    public:
        explicit Stream(int level = 0) : level_(level)
        { }

        explicit Stream(const T& channel, int level = 0)
            : level_(level)
            , channel_(channel)
        { }

        Stream& operator<<(const char_type* str)
        {
            if (channel_.level() > level_)
            {
                channel_.buffer() << str;
            }
            return *this;
        }
        template<typename U> Stream& operator<<(const U& obj)
        {
            if (channel_.level() > level_)
            {
                channel_.buffer() << obj;
            }
            return *this;
        }

        Stream& hex()
        {
            if (channel_.level() > level_)
            {
                channel_.buffer() << std::hex;
            }
            return *this;
        }
        Stream& dec()
        {
            if (channel_.level() > level_)
            {
                channel_.buffer() << std::dec;
            }
            return *this;
        }

        Stream& flush()
        {
            if (channel_.flush())
            {
                //assert(channel_.level() > level_);
            }
            return *this;
        }

        Stream& operator<<(Stream& (*pf)(Stream&))
        {
            return (*pf)(*this);
        }
    };



    template<typename T = DefaultChannel>
    struct ZDK_LOCAL Null
    {
        explicit Null(T&) { }
        Null() { }

        template<typename U> Null& operator<<(const U& obj)
        { return *this; }

        template<typename U> Null& operator<<(const U* obj)
        { return *this; }

        inline Null& operator<<(Null& (*pf)(Null&))
        {
            return (*this);
        }

    };

    template<typename T>
    inline ZDK_LOCAL Stream<T>& flush(Stream<T>& stream)
    {
        return stream.flush();
    }

    template<typename T>
    inline ZDK_LOCAL Stream<T>& endl(Stream<T>& stream)
    {
        return (stream << typename T::char_type('\n')).flush();
    }

    template<typename T>
    inline ZDK_LOCAL Null<T>& endl(Null<T>& stream)
    {
        return stream;
    }

    template<typename T>
    inline ZDK_LOCAL Stream<T>& hex(Stream<T>& stream)
    {
        return stream.hex();
    }
    template<typename T>
    inline ZDK_LOCAL Stream<T>& dec(Stream<T>& stream)
    {
        return stream.dec();
    }

    template<typename T>
    inline ZDK_LOCAL Null<T>& hex(Null<T>& stream)
    {
        return stream;
    }
    template<typename T>
    inline ZDK_LOCAL Null<T>& dec(Null<T>& stream)
    {
        return stream;
    }
}

#endif // EVENTLOG_H__5E85E497_C4D7_474D_8A58_0F7320BC43BD
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
