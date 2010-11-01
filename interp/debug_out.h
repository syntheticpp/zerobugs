#ifndef DEBUG_OUT_H__E78070C3_DF8A_4226_8B6A_08B16493F1C5
#define DEBUG_OUT_H__E78070C3_DF8A_4226_8B6A_08B16493F1C5
//
// $Id: debug_out.h 714 2010-10-17 10:03:52Z root $
//
#include <iostream>
#ifdef DEBUG
#include "interp.h"
 #define DEBUG_OUT \
    if (!Interp::debug_enabled()); \
    else clog << " (" << __func__ << ", " << __FILE__ << ":" << __LINE__ << "): "
#else
 #define DEBUG_OUT while(0) clog
#endif

#endif // DEBUG_OUT_H__E78070C3_DF8A_4226_8B6A_08B16493F1C5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
