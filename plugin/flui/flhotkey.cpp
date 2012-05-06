//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "flhotkey.h"

using namespace std;
using namespace ui;

/* static */
unique_ptr<HotKey> HotKey::alt(char key)
{
    return unique_ptr<HotKey>(new FlAltKey(key));
}

/* static */
unique_ptr<HotKey> HotKey::ctrl(char key)
{
    return unique_ptr<HotKey>(new FlCtrlKey(key));
}

/* static */
unique_ptr<HotKey> HotKey::fn(int f)
{
    return unique_ptr<HotKey>(new FlFnKey(f));
}

