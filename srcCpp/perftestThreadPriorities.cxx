/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "perftestThreadPriorities.h"

PerftestThreadPriorities::PerftestThreadPriorities()
        : main(0), receive(0), dbAndEvent(0), isSet(false)
{
    #ifdef RTI_WIN32
        defaultPriorities['h'] = THREAD_PRIORITY_TIME_CRITICAL;
        defaultPriorities['n'] = THREAD_PRIORITY_NORMAL;
        defaultPriorities['l'] = THREAD_PRIORITY_IDLE;
    #elif RTI_UNIX
        defaultPriorities['h'] = sched_get_priority_max(SCHED_FIFO);
        defaultPriorities['n'] = (sched_get_priority_max(SCHED_FIFO)
                + sched_get_priority_min(SCHED_FIFO)) / 2;
        defaultPriorities['l'] = sched_get_priority_min(SCHED_FIFO);
    #endif
}

bool PerftestThreadPriorities::set_priorities(char x, char y, char z)
{
    if (defaultPriorities.count(x) && defaultPriorities.count(y)
        && defaultPriorities.count(z)) {
        main = defaultPriorities[x];
        receive = defaultPriorities[y];
        dbAndEvent = defaultPriorities[z];
        return true;
    }
    return false;
}