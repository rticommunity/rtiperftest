/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_common.h"

#include <sys/time.h>

void *PerftestTimer::waitAndExecuteHandler(void *scheduleInfo)
{
    ScheduleInfo *info = static_cast<ScheduleInfo *>(scheduleInfo);

    PerftestClock::milliSleep(info->timer * 1000u);

    info->handlerFunction();
}

PerftestTimer &PerftestTimer::getInstance()
{
    static PerftestTimer instance;
    return instance;
}

PerftestThread* PerftestTimer::setTimeout(PerftestTimer::ScheduleInfo &info)
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
