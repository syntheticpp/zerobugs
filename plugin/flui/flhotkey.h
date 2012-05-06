#ifndef FLHOTKEY_H__33CCD424_3C6D_4108_927D_A1DE2E3F0133
#define FLHOTKEY_H__33CCD424_3C6D_4108_927D_A1DE2E3F0133
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include <FL/Enumerations.H>
#include "hotkey.h"


class FlHotKey : public ui::HotKey
{
protected:
    explicit FlHotKey(int code) : code_(code)
    { }

private:
    int to_int() const {
        return code_;
    }

    int code_;
};


struct FlAltKey : public FlHotKey
{
    explicit FlAltKey(char key) : FlHotKey(FL_ALT + key)
    { }
};


struct FlCtrlKey : public FlHotKey
{
    explicit FlCtrlKey(char key) : FlHotKey(FL_CTRL + key)
    { }
};


struct FlFnKey : public FlHotKey
{
    explicit FlFnKey(int fn) : FlHotKey(FL_F + fn)
    { }
};

#endif // FLHOTKEY_H__33CCD424_3C6D_4108_927D_A1DE2E3F0133

