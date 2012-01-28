#ifndef CONFIG_H__443DB7AA_5C16_11DA_BBFB_00C04F09BBCC
#define CONFIG_H__443DB7AA_5C16_11DA_BBFB_00C04F09BBCC
//
// $Id$
//
////////////////////////////////////////////////////////////////
#if defined (__linux__)

#include "linux_core.h"
#include "linux_live.h"

////////////////////////////////////////////////////////////////
#elif defined (__FreeBSD__)
 #include "fbsd_core.h"
 #include "fbsd_live.h"

#endif

#endif // CONFIG_H__443DB7AA_5C16_11DA_BBFB_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
