#ifndef TERMINAL_H__FE4219A4_E212_4681_B4C8_1FE035AD4ECF
#define TERMINAL_H__FE4219A4_E212_4681_B4C8_1FE035AD4ECF
//
// $Id: terminal.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdio.h>      // FILE*
#include <termios.h>    // struct termios
#include "zdk/export.h"


namespace Term
{
    enum { ESC = 27 };

    enum
    {
        KEY_BACKDEL  = 0x0008,
        KEY_TAB      = 0x0009,
        KEY_RETURN   = 0x000A,
        KEY_ENTER    = 0x000A,
        KEY_BACKSPACE= 0x007F,

        // terminal escape sequences
        KEY_UP       = 0x415B,
        KEY_DOWN     = 0x425B,
        KEY_RIGHT    = 0x435B,
        KEY_LEFT     = 0x445B,
        KEY_HOME1    = 0x485b, // PowerMac keyboard
        KEY_HOME2    = 0x484f,
        KEY_END1     = 0x465b, // PowerMac
        KEY_END2     = 0x464f,
        KEY_HOME     = 0x7E315B,
        KEY_DELETE   = 0x7E335B,
        KEY_END      = 0x7E345B,
    };

    /// Fetches attributes for given terminal, and
    /// automatically restores them when destroyed
    class ZDK_LOCAL Attr
    {
    public:
        explicit Attr(int file);

        ~Attr();

        const struct termios& get() const { return tc_; }

        void set(const struct termios&);

    private:
        Attr(const Attr&);
        Attr& operator=(const Attr&);

        int fd_;
        struct termios tc_;
    };

    /// Reads a key from the specified input file (device)
    /// If a special key (cursor movements keys, for e.g.)
    /// it returns a terminal escape sequence rather than
    /// keyboard scan codes
    int readkey(int file);

    /// Get the current width of the screen connected to
    /// the specified device
    unsigned int screen_width(int file);

    unsigned int inline screen_width(FILE *fp)
    { return screen_width(fileno(fp)); }

    unsigned int screen_height(int file);

    unsigned int inline screen_height(FILE *fp)
    { return screen_height(fileno(fp)); }

    /// move cursor by sending escape sequences to terminal,
    /// see the console_codes(4) manual page
    void move_cursor_left(FILE*, unsigned int columns = 1);

    void move_cursor_right(FILE*, unsigned int columns = 1);

    void move_cursor_up(FILE*, unsigned int rows = 1);

    void move_cursor_down(FILE*, unsigned int rows = 1);

    void save_cursor(FILE*);

    void get_cursor_pos(FILE*, unsigned int& x, unsigned int& y);

    void restore_cursor(FILE*);

    void clear_eol(FILE*);

    // -- void clear_screen(FILE*);

    void scroll_up(FILE*, unsigned int rows  = 1);

    void start_bold(FILE*);

    void end_bold(FILE*);

    void set_foreground_color(FILE*, int);
};
#endif // TERMINAL_H__FE4219A4_E212_4681_B4C8_1FE035AD4ECF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
