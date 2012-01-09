//
// $Id: thread_util.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
// Thread utility functions.
//
#include <iostream>
#include <vector>
#include <stdio.h>
#include <sys/wait.h>
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/stdexcept.h"
#include "zdk/thread_util.h"
#include "zdk/zero.h"

using namespace std;


bool thread_stopped(const Thread& thread)
{
    int status = thread.status();
    return WIFSTOPPED(status) || thread.is_exiting();
}


bool thread_exited(const Thread& thread)
{
    const int status = thread.status();
    bool result = WIFEXITED(status);

    return result;
}


int32_t thread_exit_code(const Thread& thread)
{
    int status = thread.status();
    return WEXITSTATUS(status);
}


bool thread_finished(const Thread& thread)
{
    const int status = thread.status();

    const bool result =
        WIFEXITED(status) || WIFSIGNALED(status) || WCOREDUMP(status);
#ifdef DEBUG
    if (result)
    {
        clog << __func__ << ": status=" << status << endl;
    }
#endif
    assert(result != WIFSTOPPED(status));

    return result;
}


word_t thread_peek_stack(const Thread& thread, int index)
{
    word_t result = 0;

    if (addr_t addr = thread.stack_pointer())
    {
        addr += index * sizeof(word_t);
        thread_read(thread, addr, result);

        /* // handled by thread_read
        if (thread.is_32_bit())
        {
            result &= 0xffffffff;
        } */
    }
    return result;
}


void thread_set_event_description(Thread& thread, const char* str)
{
    if (str)
    {
        RefPtr<SharedString> shstr(shared_string(str));
        thread.set_user_object(".event", shstr.get());
    }
    else
    {
        thread.set_user_object(".event", 0);
    }
}


void
thread_set_event_description(Thread& thread, RefPtr<SharedString> str)
{
    assert(str);
    thread.set_user_object(".event", str.get());
}


SharedString* thread_get_event_description(const Thread& thread)
{
    if (ZObject* userObj = thread.get_user_object(".event"))
    {
        return interface_cast<SharedString*>(userObj);
    }
    else
    {
        return NULL;
    }
}


bool thread_in_process(const Thread& thread, pid_t pid)
{
    bool result = false;

    if (Process* proc = thread.process())
    {
        result = (proc->pid() == pid);
    }
    return result;
}


void thread_set_user_breakpoint(Thread& thread, addr_t addr)
{
    thread.debugger()->set_user_breakpoint(get_runnable(&thread), addr);
}


////////////////////////////////////////////////////////////////
//
// Helpers for thread_page_find
//
static inline bool match(uint8_t a, int b)
{
    return (b == -1) ? true : (a == b);
}

static int scan(const uint8_t* buf, size_t len, int b)
{
    assert(buf);

    for (size_t i = 0; i < len; ++i)
    {
        if (match(buf[i], b))
        {
            return i;
        }
    }
    return -1;
}

static int hexdigit(int c)
{
    if (isdigit(c))
    {
        return c - '0';
    }
    if ((c >= 'a') && (c <= 'f'))
    {
        return c - 'a' + 10;
    }
    if ((c >= 'A') && (c <= 'F'))
    {
        return c - 'A' + 10;
    }
    return -1;
}

/**
 * Extract next character from pattern string
 * @return false is pattern exhausted
 */
static bool next_char(const char*& pattern, int& c)
{
    if (!*pattern)
    {
        return false;
    }
    c = *pattern++;

    switch (c)
    {
    case '?':
        c = -1;
        break;

    case '\'':
        c = *pattern;
        if (!c)
        {
            throw runtime_error("Trailing quote in pattern");
        }
        if (*++pattern != '\'')
        {
            throw runtime_error("Missing quote in pattern");
        }
        ++pattern;
        break;

    default:
        {
            int x = hexdigit(c);
            if (x == -1)
            {
                throw runtime_error("Hex number expected in pattern");
            }
            c = x;
            if (*pattern && !isspace(*pattern))
            {
                c *= 16;
                x = hexdigit(*pattern++);
                if (x == -1)
                {
                    throw runtime_error("Hex number expected in pattern");
                }
                c += x;
                if (*pattern && !isspace(*pattern))
                {
                    throw runtime_error("Extra character in hex number");
                }
            }
        }
        break;
    }
    while (isspace(*pattern))
    {
        ++pattern;
    }
    return true;
}


static const uint8_t* readbuf
(
    const Thread&   thread,
    addr_t          addr,
    vector<word_t>& buf,
    size_t&         size
)
{
    assert(buf.size());
    size_t wordsRead = 0;

    thread.read_data(addr, &buf[0], buf.size(), &wordsRead);

    size = wordsRead * sizeof (word_t);
#ifdef DEBUG
    clog << __func__ << ": " << size << " byte(s) read\n";
#endif
    return reinterpret_cast<const uint8_t*>(&buf[0]);
}


/**
 * Scan a buffer of given size for a pattern of bytes,
 * @return index of position where found, or -1.
 * Fill out a count of matching bytes.
 */
static int find
(
    const Thread& thread,
    addr_t addr,    // address in memory where the buffer was read from
    const uint8_t* buf,
    size_t size,    // size of this buffer
    const char* pattern,
    size_t& count
)
{
    assert(pattern);
    const char* searchPattern = pattern;
    const uint8_t* base = buf;

    int c = 0;
    if (!next_char(pattern, c))
    {
        return 0;// empty pattern, match at offset 0
    }

    while (size)
    {
        int n = scan(buf, size, c);
        if (n == -1)
        {
            break;
        }
        count = 1;

        for (size_t i = n + 1; i < size - 1; ++i)
        {
            if (!next_char(pattern, c))
            {
                // pattern exhausted, we got a match
                return n + buf - base;
            }
            else if (match(buf[i], c))
            {
                ++count;
            }
            else
            {
                --size, ++buf;

                count = 0, pattern = searchPattern;
                next_char(pattern, c);

                break;
            }
        }
    }
    return -1;
}


/**
 * Scan the thread's memory for a pattern of bytes -- for details
 * see comments in header file.
 */
bool thread_page_find
(
    const Thread&   thread,
    addr_t          addr,
    const char*     pattern,
    size_t*         length,
    addr_t*         retAddr
)
{
    assert(pattern);

    // size of read buffer, in machine words
    const size_t chunk = 4096 / sizeof (word_t);
    vector<word_t> buf(chunk);

    size_t size = 0, matchCount = 0;
    const uint8_t* bytes = readbuf(thread, addr, buf, size);

    int n = find(thread, addr, bytes, size, pattern, matchCount);
    if (n != -1)
    {
        if (length)
        {
            *length = matchCount;
        }
        if (retAddr)
        {
            *retAddr = addr + n;
        }
        return true;
    }
    return false;
}


Runnable* get_runnable(Thread* thread)
{
    assert(thread);

    if (Runnable* runnable = interface_cast<Runnable*>(thread))
    {
        return runnable;
    }
    throw logic_error("thread is not runnable");
}


Runnable* get_runnable(Thread* thread, std::nothrow_t)
{
    assert(thread);

    if (Runnable* runnable = interface_cast<Runnable*>(thread))
    {
        return runnable;
    }
    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
