/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_common.h"

void *PerftestTimer::waitAndExecute(void *scheduleInfo)
{
    ScheduleInfo *info = static_cast<ScheduleInfo *>(scheduleInfo);

    PerftestClock::milliSleep(info->timer * 1000u);

    if (info->handlerFunction != NULL) {
        info->handlerFunction();
    }

    return NULL;
}

PerftestTimer &PerftestTimer::getInstance()
{
    static PerftestTimer instance;
    return instance;
}

PerftestThread *PerftestTimer::setTimeout(PerftestTimer::ScheduleInfo &info)
{
    struct PerftestThread *timerThread = NULL;

    timerThread = PerftestThread_new(
            "timerThread",
            Perftest_THREAD_PRIORITY_DEFAULT,
            Perftest_THREAD_OPTION_DEFAULT,
            waitAndExecute,
            &info);

    return timerThread;
}
