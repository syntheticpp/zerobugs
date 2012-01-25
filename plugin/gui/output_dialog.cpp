//
// $Id: output_dialog.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <errno.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <memory>
#include <iostream>
#include <vector>
#include "gtkmm/main.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/text.h"
#include "dharma/system_error.h"

#include "slot_macros.h"
#include "output_dialog.h"


using namespace std;


OutputDialog::OutputDialog(const char* title, int fd, unsigned width, unsigned height)
    : TextDialog(title, width, height)
#if GTKMM_2
    , fd_(fd)
#endif
{
#if GTKMM_2
    conn_ = Glib::signal_io().connect(
        sigc::mem_fun(this, &OutputDialog::on_data), fd, Glib::IO_IN);
#else
    conn_ = Gtk::Main::input.connect(
        slot(this, &OutputDialog::on_data), fd, GDK_INPUT_READ);
#endif
}


OutputDialog::~OutputDialog()
{
    conn_.disconnect();
}


#ifdef GTKMM_2
bool OutputDialog::on_data(Glib::IOCondition cond)
{
//#ifdef DEBUG
//    clog << __func__ << ": cond=" << cond << endl;
//#endif
    char buf[512];
    vector<char> data;

    bool result = true;
    for (; /*cond == Glib::IO_IN */;)
    {
        ssize_t n = read(fd_, buf, sizeof(buf));

        if (n == 0)
        {
            result = false; // disconnect
            break;
        }
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            // pipe closed by other thread?
            if (errno != EAGAIN)
            {
                cerr << "pipe read: " << errno;
                cerr << " " << strerror(errno) << endl;
            }
            break;
        }
        else
        {
            data.insert(data.end(), buf, buf + n);
        }
    }

    if (!data.empty())
    {
        data.push_back(0); // make sure it is zero-terminated
        insert_text(&data[0]);
    }
    return result;
}

#else

BEGIN_SLOT(OutputDialog::on_data, (int fd, GdkInputCondition cond))
{
    char buf[512];
    vector<char> data;

    for (; cond == GDK_INPUT_READ;)
    {
        ssize_t n = read(fd, buf, sizeof(buf));

        if (n == 0)
        {
            conn_.disconnect();
            break;
        }

        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            // pipe closed by other thread?
            if (errno != EAGAIN)
            {
                cerr << "pipe read: " << errno;
                cerr << " " << strerror(errno) << endl;
            }
            break;
        }

        data.insert(data.end(), buf, buf + n);
    }

    if (!data.empty())
    {
        data.push_back(0); // make sure it is zero-terminated
        insert_text(&data[0]);
    }
}
END_SLOT()
#endif // !GTKMM_2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
