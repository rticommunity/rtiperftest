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

#ifdef RTI_VXWORKS
  #include <unistd.h>
#endif

/* Disable certain warnings for Windows */
#if defined(RTI_WIN32)
  #pragma warning(push)
  #pragma warning(disable : 4996)
#endif

/* Perftest Timer Class */
class PerftestTimer
{
  private:
    static void *waitAndExecuteHandler(void *timerSeconds);

  public:
    struct ScheduleInfo {
        unsigned int timer;
        void (*handlerFunction)(void);
    };

    static PerftestTimer &getInstance();
    PerftestThread *setTimeout(ScheduleInfo &info);
};

#endif /* INFRASTRUCTURE_COMMON_H_ */
