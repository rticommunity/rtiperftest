/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_common.h"

#include <chrono>

void *PerftestTimer::waitAndExecuteHandler(void *scheduleInfo) 
{
    ScheduleInfo *info = static_cast<ScheduleInfo *>(scheduleInfo);

    using namespace std::chrono;
    printf("[++++++++++] Voy a dormir %u ms\n", info->timer);
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    // Sleep for t milliseconds
    PerftestClock::milliSleep(info->timer * 1000u);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("[----------] Ya he dormido %f s\n", time_span.count());

    // Call the scheduled function with the args
    info->handlerFunction();
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
