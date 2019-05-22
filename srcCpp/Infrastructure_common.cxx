/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_common.h"

void *PerftestTimer::waitAndExecuteHandler(void *scheduleInfo) 
{
    ScheduleInfo *info = static_cast<ScheduleInfo *>(scheduleInfo);

    // Sleep until timer is reached
    #ifdef RTI_VXWORKS
      struct timespec sleepTime;
      sleepTime.tv_sec = info->timer;
      sleepTime.tv_nsec = 0;

      nanosleep(&sleepTime, NULL);
    #else
      PerftestClock::milliSleep(info->timer * 1000u);
    #endif

    // Call the scheduled function with the args
    info->handlerFunction();

    return NULL;
}

PerftestTimer &PerftestTimer::getInstance()
{
    static PerftestTimer instance;
    return instance;
}

PerftestThread* PerftestTimer::setTimeout(ScheduleInfo &info)
{
    struct PerftestThread *timerThread = NULL;

    // We have to create a new pointer to the timer so 
    // it is not removed when this function ends
    timerThread = PerftestThread_new(
            "timerThread", 
            Perftest_THREAD_PRIORITY_DEFAULT,
            Perftest_THREAD_OPTION_DEFAULT,
            waitAndExecuteHandler,
            &info);
    if (timerThread == NULL) {
        fprintf(stderr, "Problem creating timer thread.\n");
    }

    return timerThread;
}
