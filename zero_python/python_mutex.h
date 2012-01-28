#ifndef PYTHON_MUTEX_H__8186FCE1_230B_4F9F_AAA6_3640A23F00DB
#define PYTHON_MUTEX_H__8186FCE1_230B_4F9F_AAA6_3640A23F00DB
//
// $Id$
//
#include "zdk/mutex.h"

/**
 * @return a mutex that needs to be locked while executing
 * commands on the main thread
 */
Mutex& python_mutex();


#endif // PYTHON_MUTEX_H__8186FCE1_230B_4F9F_AAA6_3640A23F00DB
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
