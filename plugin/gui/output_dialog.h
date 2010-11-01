#ifndef OUTPUT_DIALOG_H__BC410F2B_2991_412C_88E3_834E3739111E
#define OUTPUT_DIALOG_H__BC410F2B_2991_412C_88E3_834E3739111E
//
// $Id: output_dialog.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "text_dialog.h"


class OutputDialog : public TextDialog
{
public:
    OutputDialog(const char* title, int fd, unsigned width = 480, unsigned height = 250);

    ~OutputDialog();

private:
    SigC::Connection    conn_;
#if GTKMM_2
    int                 fd_;

    bool on_data(Glib::IOCondition);

#else
    void on_data(int fd, GdkInputCondition);
#endif
};

#endif // OUTPUT_DIALOG_H__BC410F2B_2991_412C_88E3_834E3739111E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
