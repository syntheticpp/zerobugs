#ifndef THREAD_UTIL_H__6BFD8FDD_AEFC_4646_B4E0_B31B6139FF1D
#define THREAD_UTIL_H__6BFD8FDD_AEFC_4646_B4E0_B31B6139FF1D
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Convenience functions, implemented in terms of public
// methods of class Thread. See zdk/zero.h for definition
// of the Thread interface.
//
#include "zdk/config.h"
#include <vector>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pod.hpp>
#include "zdk/align.h"
#include "zdk/ref_ptr.h"
#include "zdk/32_on_64.h"


struct SharedString;


/**
 * @return true if thread has stopped because of a signal.
 */
bool ZDK_LOCAL thread_stopped(const Thread&);

/**
 * @return true if thread exited normally
 */
bool ZDK_LOCAL thread_exited(const Thread&);

int ZDK_LOCAL thread_exit_code(const Thread&);

/**
 * @return true if exited normally or because of uncaught signal
 *
 * @note WIFEXITED() (see waitpid) implies normal exit,
 * WIFSIGNALED means killed by a signal, and `terminated'
 * might be confused with `received a SIGTERM'.
 */
bool ZDK_LOCAL thread_finished(const Thread&);

/**
 * call waitpid(), then update status
 */
void ZDK_LOCAL thread_wait(Thread&);

/**
 * read an int from the thread's stack, at given index
 */
word_t ZDK_LOCAL thread_peek_stack(const Thread&, int index = 0);

void ZDK_LOCAL thread_set_event_description(Thread& thread, const char*);

ZDK_LOCAL SharedString* thread_get_event_description(const Thread& thread);

void ZDK_LOCAL
thread_set_event_description(Thread& thread, RefPtr<SharedString>);

inline bool ZDK_LOCAL thread_is_attached(Thread& thread)
{
    Process* process = thread.process();
    return process ? process->is_attached(&thread) : false;
}


/**
 * @return true if the specified thread belongs to the process
 * identified by the given id.
 */
bool ZDK_LOCAL thread_in_process(const Thread&, pid_t);

void ZDK_LOCAL thread_set_user_breakpoint(Thread&, addr_t);


////////////////////////////////////////////////////////////////
// thread_read
namespace detail
{
    /**
     * helpers for thread_read
     */
    template<int Remainder> struct ZDK_LOCAL ReadMemImpl
    {
        template<typename T>
        void operator()(const Thread& t, addr_t a, T& v, size_t* np)
        {
            size_t nwords = round_to_word(sizeof(T));
            std::vector<word_t> buf(nwords);
            t.read_data(a, &buf[0], nwords, np);
            memcpy(&v, &buf[0], sizeof(T));
        }
    };

    template<> struct ZDK_LOCAL ReadMemImpl<0>
    {
        template<typename T>
        void operator()(const Thread& t, addr_t a, T& v, size_t* np)
        {
            const size_t nwords = sizeof(T) / sizeof(word_t);
            t.read_data(a, (word_t*)&v, nwords, np);
        }
    };
}

template<typename T> struct ZDK_LOCAL ReadMem
{
    void operator()(const Thread& t, addr_t a, T& v, size_t* np = 0)
    {
        detail::ReadMemImpl<sizeof(T) % sizeof(word_t)>()(t, a, v, np);
        Platform::after_read(t, v);
    }
};


template<typename T>
inline bool ZDK_LOCAL
thread_read(const Thread& thread, addr_t addr, T& t, size_t* np=0)
{
    //boost::is_POD seems to be broken here?
    //BOOST_STATIC_ASSERT(boost::is_POD<T>::value);

    ReadMem<T>()(thread, addr, t, np);
    return np ? *np : true;
}

// Cross-read
template<int W, template<int> class X>
inline void ZDK_LOCAL
thread_xread(const Thread& t, addr_t addr, X<__WORDSIZE>& x, size_t* np)
{
    if (W == __WORDSIZE)
    {
        ReadMem<X<__WORDSIZE> >()(t, addr, x, np);
    }
    else
    {
        X<W> tmp;
        ReadMem<X<W> >()(t, addr, tmp, np);

        x = tmp;
    }
}

template<template<int> class X>
inline void ZDK_LOCAL
thread_readx(const Thread& t, addr_t addr, X<__WORDSIZE>& x, size_t* np = 0)
{
    if ((__WORDSIZE != 32) && t.is_32_bit())
    {
        thread_xread<32>(t, addr, x, np);
    }
    else
    {
        thread_xread<__WORDSIZE>(t, addr, x, np);
    }
}

template<typename T>
inline void ZDK_LOCAL thread_poke_word(Thread& thread, addr_t addr, T value)
{
    if ((sizeof(T) < sizeof(word_t))
            || ((__WORDSIZE != 32) && thread.is_32_bit()))
    {
        union
        {
            word_t word;
            int32_t val;
        } un;
        thread.read_data(addr, &un.word, 1);
        un.val = value;

        thread.write_data(addr, &un.word, 1);
    }
    else
    {
        thread.write_data(addr, &value, 1);
    }
}


////////////////////////////////////////////////////////////////

/**
 * Search thread's memory for a sequence of bytes.
 * The byte pattern is described by a string of the following format:
 * @code
 * BYTESPEC := BYTE | BYTESPEC BYTE
 * BYTE := 'CHAR' | HEXNUMBER | WILDCARD
 * CHAR := a-z | A-Z
 * HEXNUMBER := HEXDIGIT HEXDIGIT
 * HEXDIGIT := 0-9 | a-f | A-F
 * WILDCARD := ?
 * @endcode
 * The * wildcard matches any number of bytes, ? matches exactly one byte.
 * Example: 'a' 42 E5 f4 'X' 'Y' 'Z' * 12
 * @param thread
 * @param addr address where to begin the search
 * @param bytespec a string that specifies the byte pattern to search for.
 * @param upon return, if not NULL, it will be filled out with the
 *  length of the matching sequence (useful when bytespec has wildcards)
 * If NULL, this parameter is ignored.
 * @param retAddr if not NULL, upon return will contain the address where
 * the byte pattern was found.
 * @return true if the given byte pattern was found, or false otherwise.
 */
ZDK_LOCAL bool thread_page_find
(
    const Thread&   thread,
    addr_t          addr,
    const char*     bytespec,
    size_t*         length = 0, /* in-out */
    addr_t*         retAddr = 0 /* out */
);


/**
 * @return current (selected) frame, or NULL
 */
ZDK_LOCAL Frame* thread_current_frame(const Thread*);

/**
 * @return the function at the currently selected frame, or NULL
 */
ZDK_LOCAL Symbol* thread_current_function(const Thread*);

/**
 * @return the symbol table at the currently selected frame, or NULL
 */
ZDK_LOCAL RefPtr<SymbolTable> thread_current_symbol_table(const Thread*);


/**
 * @return the address where local variables for the given
 * function start, which sometimes may be off from the value
 * in frame.frame_pointer() as it is the case of code compiled
 * with GCC 4.1.0 on the i386; functions may have a prologue that
 * aligns the framebase to multiple of 16. The alignment does not
 * apply to function parameters, just to local variables. In the
 * case of function main(), it appears that argc and argv are not
 * subject to the alignment (as they are parameters), but the
 * pointer to the environment and the auxiliary vector are.
 */
ZDK_LOCAL addr_t thread_frame_base(Thread&, Frame&, Symbol& func);


#endif // THREAD_UTIL_H__6BFD8FDD_AEFC_4646_B4E0_B31B6139FF1D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
