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
#define PerftestSemaphore_give RTIOsapiSemaphore_give
#define PERFTEST_SEMAPHORE_TIMEOUT_INFINITE -1

inline bool PerftestSemaphore_take(PerftestSemaphore *sem, int timeout)
{
    struct RTINtpTime block_duration = RTI_NTP_TIME_MAX;
    if (timeout != PERFTEST_SEMAPHORE_TIMEOUT_INFINITE) {
        RTINtpTime_packFromMillisec(block_duration, 0, timeout);
    }
    return RTIOsapiSemaphore_take(sem, &block_duration)
            == RTI_OSAPI_SEMAPHORE_STATUS_ERROR
            ? false
            : true;
}

#define Perftest_param_not_micro(param)
#define Perftest_not_supported_in_micro(message)

/* Perftest Clock Class */
class PerftestClock
{

    private:
        static struct RTIClock *Clock;
        static struct RTINtpTime ClockTime_aux;
        static RTI_UINT64 Clock_sec;
        static RTI_UINT64 Clock_usec;

    public:
        static void Initialize();
        static void Finalize();
        static unsigned long long GetTimeUsec();

        static void MilliSleep(unsigned int millisec) {
          #if defined(RTI_WIN32)
            Sleep(millisec);
          #elif defined(RTI_VXWORKS)
            DDS_Duration_t sleep_period = {0, millisec*1000000};
            NDDSUtility::sleep(sleep_period);
          #else
            usleep(millisec * 1000);
          #endif
        }
};

bool configureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos);

void PerftestCreateThread(
        const char *name,
        RTIOsapiThreadOnSpawnedMethod method,
        void *threadParam);

void configureVerbosity(int verbosityLevel);

#endif /* INFRASTRUCTURE_PRO_H_ */
