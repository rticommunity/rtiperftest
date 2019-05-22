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
    double elapsedTime, targetTime;
    struct timeval t_orig, t_now;

    gettimeofday(&t_orig, NULL);
    targetTime = t_orig.tv_sec + info->timer;

    printf("[++++++++++] Voy a dormir %u s\n", info->timer);   
    printf("[#####] Now %f s\n", t_orig.tv_sec);
    printf("[#####] Target %f s\n", targetTime);

    // Sleep until timer is reached
    #ifdef RTI_VXWORKS
      do {
          PerftestClock::milliSleep(info->timer * 1000u);
          gettimeofday(&t_now, NULL);

          printf("\t[#######] Dormido %f s\n", t_now.tv_sec - t_orig.tv_sec);
          printf("\t[#######] Restante %f s\n", targetTime - t_now.tv_sec);
      } while (t_now.tv_sec < targetTime);
    #else
      PerftestClock::milliSleep(info->timer * 1000u);
    #endif

    gettimeofday(&t_now, NULL);
    elapsedTime = (t_now.tv_sec - t_orig.tv_sec);
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
