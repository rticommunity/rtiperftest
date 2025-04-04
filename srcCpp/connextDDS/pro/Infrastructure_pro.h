/*
 * (c) 2005-2019 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_PRO_H_
#define INFRASTRUCTURE_PRO_H_

#include "clock/clock_highResolution.h"
#include "osapi/osapi_ntptime.h"
#include "osapi/osapi_process.h"
#include "ndds/ndds_cpp.h"

/*
 * In order to enable/disable certain features depending on the version of
 * ConnextDDS Pro, we will use the version number of the middleware:
 *
 * Version list:
 * Ironside -- 7.0.0 -- PERFTEST_CONNEXT_PRO_700
 * Hercules -- 6.1.0 -- PERFTEST_CONNEXT_PRO_610
 */

#if RTI_DDS_VERSION_MAJOR >= 6 && RTI_DDS_VERSION_MINOR >= 1
  #define PERFTEST_CONNEXT_PRO_610
#endif


/*
 * The way in which we do this is the following: If the minor version is == X
 * we will define *_PRO_7X0 and every other version below X.
 */
#if RTI_DDS_VERSION_MAJOR >= 7
  // 6.1.X features are also in 7.0.0
  #define PERFTEST_CONNEXT_PRO_610
  #define PERFTEST_CONNEXT_PRO_700
  #if RTI_DDS_VERSION_MINOR >= 1
    #define PERFTEST_CONNEXT_PRO_710
  #endif
  #if RTI_DDS_VERSION_MINOR >= 2
    #define PERFTEST_CONNEXT_PRO_720
  #endif
  #if RTI_DDS_VERSION_MINOR >= 3
    #define PERFTEST_CONNEXT_PRO_730
  #endif
  #if RTI_DDS_VERSION_MINOR >= 4
    #define PERFTEST_CONNEXT_PRO_740
  #endif
  #if RTI_DDS_VERSION_MINOR >= 5
    #define PERFTEST_CONNEXT_PRO_750
  #endif
#endif

#include "PerftestTransport.h"

#ifdef RTI_SECURE_PERFTEST
  #include "security/security_default.h"
  #include "PerftestSecurity.h"
#endif

#include <sstream>

#ifndef RTI_USE_CPP_11_INFRASTRUCTURE

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
#define ONE_BILLION  1000000000L // 1 billion (US) == 1 second in ns

/* Perftest Clock Class */
class PerftestClock
{

  private:
  #ifndef RTI_PERFTEST_NANO_CLOCK
    RTIClock *clock;
    RTINtpTime clockTimeAux;
    RTI_UINT64 clockSec;
    RTI_UINT64 clockUsec;
  #else
    struct timespec timeStruct;
  #endif

  public:
    PerftestClock();
    ~PerftestClock();

    static PerftestClock &getInstance();
    unsigned long long getTime();
    static void milliSleep(unsigned int millisec);
    static void sleep(const struct DDS_Duration_t& sleep_period);

};

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

#endif //#ifndef RTI_USE_CPP_11_INFRASTRUCTURE

/*
 * After 7.0.0 we have changed the name of some RTI utility functions. The following
 * code is to be able to keep compatibility between 7.0.0, 6.1.X and everything
 * that will come after. The trick is that now we have a "osapi_file_h", so we should
 * be able to check if it was loaded or not already.
 */
#ifdef osapi_file_h
  #define PerftestFile_remove RTIOsapiFile_remove
  #define PerftestFile_exists RTIOsapiFile_exists
#else
  #define PerftestFile_remove RTIOsapi_removeFile
  #define PerftestFile_exists RTIOsapiUtility_fileExists
#endif //osapi_file_h

/********************************************************************/
/* Transport Related functions */

bool PerftestConfigureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos,
        ParameterManager *parameterManager);

#ifdef RTI_SECURE_PERFTEST
bool PerftestConfigureSecurity(
        PerftestSecurity &security,
        DDS_DomainParticipantQos &qos,
        ParameterManager *parameterManager);
#endif

#endif /* INFRASTRUCTURE_PRO_H_ */
