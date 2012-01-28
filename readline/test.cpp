// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

class ndelay
{
public:
    explicit ndelay(int fd) : fd_(fd), flags_(0)
    {
        flags_ = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags_ | O_NDELAY);
    }

    ~ndelay() { fcntl(fd_, F_SETFL, flags_); }

private:
    ndelay(const ndelay&);
    ndelay& operator=(const ndelay&);

    int fd_;
    int flags_;
};


int getc(int fd)
{
    char c = 0;
    int nchars = 0;

    //ioctl(fd, FIONREAD, &nchars);
    //fprintf(stderr, "%s: nchars=%d\n", __func__, nchars);
    //if (nchars)
    {
        //ndelay n(fd);

        nchars = read(fd, &c, 1);
        if (nchars < 0 && errno != EAGAIN)
        {
            fprintf(stderr, "%s: %s\n", __func__, strerror(errno));
        }
        if (nchars == 0)
        {
            return EOF;
        }
    }
    return c;
}

int main()
{
    struct winsize ws;
    struct termios ts;

    ioctl(1, TIOCGWINSZ, &ws);
    fprintf(stderr, "winsize=%dx%d\n", ws.ws_col, ws.ws_row);


    tcgetattr(1, &ts);

    ts.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(1, TCSANOW, &ts);

    int c;

    while ((c = getc(1)) != -1)
    {
        if (isprint(c))
        {
            fprintf(stderr, "%c", toupper(c));
        }
        else
        {
            if (c == 27)
            {
                int x = 0;
                read(1, &x, sizeof(int));
                fprintf(stderr, " %x ", x);
            }
            else
                fprintf(stderr, "%d", c);
        }
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
