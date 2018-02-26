/*
 * (c) 2005-2018 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_COMMON_H_
#define INFRASTRUCTURE_COMMON_H_

#ifndef RTI_MICRO
  #include "Infrastructure_pro.h"
#endif

#ifndef RTI_WIN32
  #include <sys/time.h>
  #include <unistd.h>
  #include <sched.h>
  #include <fcntl.h>
  #include <signal.h>
#endif

/* STRNCASECMP and IS_OPTION */
#if defined(RTI_WIN32)
#pragma warning(push)
#pragma warning(disable : 4996)
#define STRNCASECMP _strnicmp
#elif defined(RTI_VXWORKS)
#define STRNCASECMP strncmp
#else
#define STRNCASECMP strncasecmp
#endif
#define IS_OPTION(str, option) (STRNCASECMP(str, option, strlen(str)) == 0)

#ifndef RTI_WIN32
typedef void (*sighandler_t)(int);
#endif

/* Perftest Timer Class */
class PerftestTimer {

private:
#ifdef RTI_WIN32
    static HANDLE _hTimerQueue;
    static HANDLE _hTimer;
    static LARGE_INTEGER _ClockFrequency;
#endif
    static void (*handler_function)(void);

public:
    static void Initialize();
    static void Finalize();
    static void SetTimeout(unsigned int executionTimeInSeconds,
            void (*function)(void));
#ifdef RTI_WIN32
    static VOID CALLBACK TimeoutTask(PVOID lpParam, BOOLEAN timerOrWaitFired);
#else
    static void TimeoutTask(int sign);
#endif

};

bool configureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos);

#endif /* INFRASTRUCTURE_COMMON_H_ */
