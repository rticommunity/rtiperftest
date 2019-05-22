/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_common.h"

#include <sys/time.h>

void *PerftestTimer::waitAndExecuteHandler(void *scheduleInfo) 
{
    ScheduleInfo *info = static_cast<ScheduleInfo *>(scheduleInfo);
    const double timerThreshold = 1;
    struct timeval t1, t2;
    double elapsedTime;


    printf("[++++++++++] Voy a dormir %u s\n", info->timer);
    struct timeval t_orig;
    gettimeofday(&t_orig, NULL);

    // Sleep until timer is reached
    #ifdef RTI_VXWORKS
      do {
          gettimeofday(&t1, NULL);
          PerftestClock::milliSleep(info->timer * 1000u);
          gettimeofday(&t2, NULL);
          elapsedTime = (t2.tv_sec - t1.tv_sec);

          printf("\t[#######] Dormido %f s\n", elapsedTime);
          printf("\t[#######] Restante %f s\n", info->timer - elapsedTime);
      } while (info->timer - elapsedTime > timerThreshold);
    #else
      PerftestClock::milliSleep(info->timer * 1000u);
    #endif

    gettimeofday(&t2, NULL);
    elapsedTime = (t2.tv_sec - t_orig.tv_sec);
    printf("[----------] Ya he dormido %f s\n", elapsedTime);

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
