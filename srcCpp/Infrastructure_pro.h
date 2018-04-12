/*
 * (c) 2005-2018 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_PRO_H_
#define INFRASTRUCTURE_PRO_H_

#include "clock/clock_highResolution.h"
#include "osapi/osapi_ntptime.h"
#include "ndds/ndds_cpp.h"
#include "PerftestTransport.h"

#ifdef RTI_WIN32
  #include <windows.h>
#else
  #include <sys/time.h>
  #include <unistd.h>
  #include <sched.h>
  #include <fcntl.h>
  #include <signal.h>
#endif

/*
 * In order to unify the implementations for Micro and Pro, we wrap the
 * semaphores to a common PerftestSemaphore implementation.
 */

#define PerftestSemaphore RTIOsapiSemaphore
#define PerftestSemaphore_new() RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL)
#define PerftestSemaphore_delete RTIOsapiSemaphore_delete
#define PERFTEST_SEMAPHORE_TIMEOUT_INFINITE -1

inline bool PerftestSemaphore_take(PerftestSemaphore *sem, int timeout)
{
    struct RTINtpTime block_duration = RTI_NTP_TIME_MAX;
    if (timeout != PERFTEST_SEMAPHORE_TIMEOUT_INFINITE) {
        RTINtpTime_packFromMillisec(block_duration, 0, timeout);
    }
    /*
     * RTIOsapiSemaphore_take can return 3 values:
     * - OK
     * - TIMEOUT
     * - ERROR
     * We will only return false if ERROR
     */
    return RTIOsapiSemaphore_take(sem, &block_duration) != RTI_OSAPI_SEMAPHORE_STATUS_ERROR;
}

inline bool PerftestSemaphore_give(PerftestSemaphore *sem)
{
    return RTIOsapiSemaphore_give(sem) == RTI_OSAPI_SEMAPHORE_STATUS_OK;
}

#define PerftestLogParamNotMicro(param)
#define PerftestLogNotSupportedInMicro(message)

/* Perftest Clock Class */
class PerftestClock
{

  private:
    RTIClock *clock;
    RTINtpTime clock_time_aux;
    RTI_UINT64 clock_sec;
    RTI_UINT64 clock_usec;

  public:
    PerftestClock();
    ~PerftestClock();

    static PerftestClock &GetInstance();
    unsigned long long GetTimeUsec();
    static void MilliSleep(unsigned int millisec);
};

bool PerftestConfigureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos);

bool PerftestCreateThread(
        const char *name,
        RTIOsapiThreadOnSpawnedMethod method,
        void *threadParam);

void PerftestConfigureVerbosity(int verbosityLevel);

#endif /* INFRASTRUCTURE_PRO_H_ */
