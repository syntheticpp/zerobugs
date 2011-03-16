//
// $Id: terminal.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/config.h"
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#ifdef HAVE_TERMCAP_H
 #include <termcap.h>
#endif
#include <map>
#include <stdexcept>
#include <string>
#include "generic/temporary.h"
#include "terminal.h"
#include "dharma/redirect.h"
#include "dharma/system_error.h"


#define _TERM_USE_ESCAPE 1

// todo: always termcap rather than hard-coded escape codes

using namespace std;

static const unsigned int DEFAULT_SCREEN_WIDTH = 78;
static const unsigned int DEFAULT_SCREEN_HEIGHT = 25;


namespace Term
{
    void init();
}


Term::Attr::Attr(int fd) : fd_(fd)
{
    if (tcgetattr(fd, &tc_) < 0)
    {
        // todo: should not throw if input comes from
        // a pipe rather than the actual terminal, how
        // do I handle errors best?
        throw SystemError("tcgetattr");
    }
}


Term::Attr::~Attr()
{
    tcsetattr(fd_, TCSADRAIN, &tc_);
}


void Term::Attr::set(const struct termios& tc)
{
    if (tcsetattr(fd_, TCSADRAIN, &tc) < 0)
    {
        throw SystemError("tcsetattr");
    }
}

#if !_TERM_USE_ESCAPE
// termcap stuff
static bool initialized = false;
static char tc_buf[2048];

static string tc_LE;    // move cursor left
static string tc_RI;    // move cursor right
static string tc_UP;    // move cursor up
static string tc_DO;    // move cursor down
static string tc_sc;    // save cursor position
static string tc_rc;    // restore cursor position
static string tc_ce;    // clear to eol
static string tc_cs;    // scroll
static string tc_md;    // bold
static string tc_me;    // end bold, etc.
//static string tc_AF;    // fg color

#define TGETSTR(n)                                          \
    if (char* p = tgetstr(#n, NULL))                        \
    {                                                       \
        tc_##n = p;                                         \
        free(p);                                            \
    }                                                       \
    else                                                    \
    {                                                       \
        throw runtime_error("termcap not found: " #n);      \
    }

static FILE* tc_out = 0;


static int tc_putchar(int c)
{
    assert(tc_out);
    return putc(c, tc_out);
}


// wrappers around tputs -- see termcap.h

static void tputs(FILE* file, const string& cmd)
{
    Term::init();

    Temporary<FILE*>__(tc_out, file);
    tparam(cmd.c_str(), tc_buf, sizeof(tc_buf));
    tputs(tc_buf, 1, tc_putchar);
}


static void tputs(FILE* file, const string& cmd, unsigned int param)
{
    Term::init();

    Temporary<FILE*>__(tc_out, file);
    tparam(cmd.c_str(), tc_buf, sizeof(tc_buf), param);
    tputs(tc_buf, 1, tc_putchar);
}
#endif


void Term::init()
{
#if !_TERM_USE_ESCAPE
    if (initialized)
    {
        return;
    }
    const char* term = getenv("TERM");

    if (!term)
    {
        throw runtime_error("TERM environment variable not set");
    }
    int tc = tgetent(NULL, term);
    if (tc <= 0)
    {
        throw runtime_error("termcap entry not found for " + string(term));
    }

    TGETSTR(LE);
    TGETSTR(RI);
    TGETSTR(UP);
    TGETSTR(DO);
    TGETSTR(sc);
    TGETSTR(rc);
    TGETSTR(ce);
    TGETSTR(cs);

    TGETSTR(md);
    TGETSTR(me);

    //TGETSTR(AF);

    initialized = true;
#endif // _TERM_USE_ESCAPE
}


unsigned int Term::screen_width(int file)
{
    struct winsize ws = { 0, 0 };

    while (ioctl(file, TIOCGWINSZ, &ws) < 0)
    {
        if (errno != EINTR)
        {
            return static_cast<unsigned int>(-1);
        }
    }
    return ws.ws_col ? ws.ws_col : DEFAULT_SCREEN_WIDTH;
}


unsigned int Term::screen_height(int file)
{
    struct winsize ws = { 0, 0 };

    while (ioctl(file, TIOCGWINSZ, &ws) < 0)
    {
        if (errno != EINTR)
        {
            return static_cast<unsigned int>(-1);
        }
    }
    return ws.ws_row ? ws.ws_row : DEFAULT_SCREEN_HEIGHT;
}


int Term::readkey(int file)
{
    int key = 0;

    char c = 0;
    int  n = 0;

    while ((n = read(file, &c, 1)) < 0)
    {
        if (errno != EINTR) throw SystemError(__func__);
    }

    if (n == 0)
    {
        key = EOF;
    }
    else if (c == ESC)
    {
        while ((n = read(file, &key, sizeof key)) < 0)
        {
            if (errno != EINTR) throw SystemError(__func__);
        }
#if __BYTE_ORDER == __BIG_ENDIAN
        ByteSwap<32>::apply(key);
#endif
    }
    else
    {
        key = c;
    }
    return key;
}


// see console_codes(4) manual page for escape codes

void Term::move_cursor_left(FILE* file, unsigned int columns)
{
    if (columns)
    {
#if _TERM_USE_ESCAPE
        fprintf(file, "\033[%dD", columns);
#else
        tputs(file, tc_LE, columns);
#endif
        fflush(file);
    }
}


void Term::move_cursor_right(FILE* file, unsigned int columns)
{
    if (columns)
    {
#if _TERM_USE_ESCAPE
        fprintf(file, "\033[%dC", columns);
#else
        tputs(file, tc_RI, columns);
#endif
        fflush(file);
    }
}


void Term::move_cursor_up(FILE* file, unsigned int rows)
{
    if (rows)
    {
#if _TERM_USE_ESCAPE
        fprintf(file, "\033[%dA", rows);
#else
        tputs(file, tc_UP, rows);
#endif
        fflush(file);
    }
}

void Term::move_cursor_down(FILE* file, unsigned int rows)
{
    if (rows)
    {
#if _TERM_USE_ESCAPE
        fprintf(file, "\033[%dB", rows);
#else
        tputs(file, tc_DO, rows);
#endif
        fflush(file);
    }
}


void Term::save_cursor(FILE* file)
{
#if _TERM_USE_ESCAPE
    fprintf(file, "\0337");
#else
    tputs(file, tc_sc);
#endif
    fflush(file);
}


void Term::restore_cursor(FILE* file)
{
#if _TERM_USE_ESCAPE
    fprintf(file, "\0338");
#else
    tputs(file, tc_rc);
#endif
    fflush(file);
}


void Term::get_cursor_pos(FILE* file, unsigned& x, unsigned& y)
{
    Term::Attr ttyAttr(STDIN_FILENO);

    // temporarily disable echoing and buffering
    termios tc = ttyAttr.get();
    tc.c_lflag &= ~(ECHO | ICANON);

    ttyAttr.set(tc);

    fprintf(file, "\033[6n");
    fflush(file);

    string buf;

    int n = 0;
    for (char c = 0; c != 'R';)
    {
        while ((n = read(STDIN_FILENO, &c, 1)) < 0)
        {
            if (errno != EINTR) throw SystemError(__func__);
        }
        buf += c;
    }

    sscanf(buf.c_str(), "\033[%d;%dR", &y, &x);
}


void Term::clear_eol(FILE* file)
{
#if _TERM_USE_ESCAPE
    fprintf(file, "\033[K");
#else
    tputs(file, tc_ce);
#endif
    fflush(file);
}


void Term::scroll_up(FILE* file, unsigned int rows)
{
    for (unsigned int n = 0; n != rows; ++n)
    {
#if 1 // _TERM_USE_ESCAPE
        fprintf(file, "\033[K\033M"); // clear to eol and scroll
#else
        tputs(file, tc_cs, 1, screen_height(file));
#endif
    }
    fflush(file);
}



void Term::set_foreground_color(FILE* file, int fg)
{
#if 1//_TERM_USE_ESCAPE
    fprintf(file, "\033[3%dm", fg);
#else
    init();

    tputs(file, tc_AF, fg);
#endif
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
