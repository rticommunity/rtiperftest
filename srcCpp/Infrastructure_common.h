/*
 * (c) 2005-2019 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_COMMON_H_
#define INFRASTRUCTURE_COMMON_H_

#include "perftest.h"

#ifdef RTI_MICRO
  #include "Infrastructure_micro.h"
#else
  #include "Infrastructure_pro.h"
#endif

/* Disable certain warnings for Windows */
#if defined(RTI_WIN32)
  #pragma warning(push)
  #pragma warning(disable : 4996)
#endif

#ifndef RTI_WIN32
  #include <sys/time.h>
  #include <unistd.h>
  #include <sched.h>
  #include <fcntl.h>
  #include <signal.h>
#endif

/* Perftest Timer Class */
class PerftestTimer
{

  private:
  #ifdef RTI_WIN32
    static HANDLE _hTimerQueue;
    static HANDLE _hTimer;
    static LARGE_INTEGER _ClockFrequency;
  #endif
    static void (*_handlerFunction)(void);
    static void *waitAndExecuteHandler(void *timer);

  public:
    PerftestTimer();
    ~PerftestTimer();
    static PerftestTimer &getInstance();
    void setTimeout(unsigned int executionTimeInSeconds, void (*function)(void));
  #ifdef RTI_WIN32
    static VOID CALLBACK timeoutTask(PVOID lpParam, BOOLEAN timerOrWaitFired);
  #else
    static void timeoutTask(int sign);
  #endif
};

#endif /* INFRASTRUCTURE_COMMON_H_ */
