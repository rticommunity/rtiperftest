/*
 * (c) 2005-2019 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_PRO_H_
#define INFRASTRUCTURE_PRO_H_

#include "clock/clock_highResolution.h"
#include "osapi/osapi_ntptime.h"
#include "ndds/ndds_cpp.h"
#include "PerftestTransport.h"

#ifdef RTI_SECURE_PERFTEST
  #include "security/security_default.h"
  #include "PerftestSecurity.h"
#endif

#include <sstream>

/*
 * In order to unify the implementations for Micro and Pro, we wrap the
 * semaphores to a common PerftestSemaphore implementation.
 */

#define PerftestSemaphore RTIOsapiSemaphore
#define PerftestSemaphore_new() \
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL)
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

/*
 * Implementation for Mutex. Since in Pro it is based in semaphore, we will use
 * the existing implementation of the functions we have created for
 * PerftestSemaphore
 */

#define PerftestMutex RTIOsapiSemaphore
#define PerftestMutex_new() \
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_MUTEX, NULL)
#define PerftestMutex_delete RTIOsapiSemaphore_delete
#define PerftestMutex_give PerftestSemaphore_give
#define PerftestMutex_take(sem) \
        PerftestSemaphore_take(sem, PERFTEST_SEMAPHORE_TIMEOUT_INFINITE)

#define PERFTEST_DISCOVERY_TIME_MSEC 1000 // 1 second

/* Perftest Clock Class */
class PerftestClock
{

  private:
    RTIClock *clock;
    RTINtpTime clockTimeAux;
    RTI_UINT64 clockSec;
    RTI_UINT64 clockUsec;

  public:
    PerftestClock();
    ~PerftestClock();

    static PerftestClock &getInstance();
    unsigned long long getTimeUsec();
    static void milliSleep(unsigned int millisec);

};

const std::string GetDDSVersionString();

void PerftestConfigureVerbosity(int verbosityLevel);


/********************************************************************/
/* THREADS */

#define PerftestThread RTIOsapiThread
#define PerftestThread_delete RTIOsapiThread_delete

#define Perftest_THREAD_PRIORITY_DEFAULT RTI_OSAPI_THREAD_PRIORITY_DEFAULT
#define Perftest_THREAD_SETTINGS_REALTIME_PRIORITY DDS_THREAD_SETTINGS_REALTIME_PRIORITY
#define Perftest_THREAD_SETTINGS_PRIORITY_ENFORCE DDS_THREAD_SETTINGS_PRIORITY_ENFORCE
#define Perftest_THREAD_OPTION_DEFAULT RTI_OSAPI_THREAD_OPTION_DEFAULT

struct PerftestThread* PerftestThread_new(
        const char *name,
        int threadPriority,
        int threadOptions,
        RTIOsapiThreadOnSpawnedMethod method,
        void *threadParam);


/********************************************************************/
/* Transport Related functions */

bool PerftestConfigureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos,
        ParameterManager *_PM);

#ifdef RTI_SECURE_PERFTEST
bool PerftestConfigureSecurity(
        PerftestSecurity &security,
        DDS_DomainParticipantQos &qos,
        ParameterManager *_PM);
#endif

#endif /* INFRASTRUCTURE_PRO_H_ */
