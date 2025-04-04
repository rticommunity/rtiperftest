/*
 * (c) 2023-2024  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifdef PERFTEST_CERT

#include "Infrastructure_common.h"

#ifdef RTI_QNX
#include <sys/neutrino.h>
#include <sys/syspage.h>
#endif

#ifndef RTI_USE_CPP_11_INFRASTRUCTURE

/********************************************************************/
/* Perftest Clock class */

PerftestClock::PerftestClock()
{
  #ifndef RTI_PERFTEST_NANO_CLOCK
  #ifndef RTI_WIN32

    OSAPI_NtpTime_from_millisec(&clockTimeAux, 0, 0);
    clockSec = 0;
    clockUsec = 0;

  #else
    _frequency = 0.0;
    LARGE_INTEGER ticks;
    if(!QueryPerformanceFrequency(&ticks)){
        printf("QueryPerformanceFrequency failed!\n");
    }

    _frequency = double(ticks.QuadPart);

  #endif
  #endif
}

PerftestClock::~PerftestClock()
{
}

PerftestClock &PerftestClock::getInstance()
{
    static PerftestClock instance;
    return instance;
}

unsigned long long PerftestClock::getTime()
{
  #ifndef RTI_PERFTEST_NANO_CLOCK
    #ifndef RTI_WIN32

    #if defined(RTI_QNX)

    // Get cycles per second for this machine
    uint64_t cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;

    // Get current timestamp in CPU cycles instead of time
    uint64_t now = ClockCycles();

    clockSec = now / cps;
    clockUsec = (now % cps) * 1000000 / cps;

    return clockUsec + clockSec * 1000000;

    #else

    if (!OSAPI_System_get_time((OSAPI_NtpTime*)&clockTimeAux)) {
        return 0;
    }

    /* OSAPI_NtpTime_to_microsec is not available in CERT library */
    OSAPI_NtpTime_to_nanosec(
            &clockSec,
            &clockUsec,
            (struct OSAPI_NtpTime*)&clockTimeAux);
    clockUsec = clockUsec / 1000;
    return clockUsec + (unsigned long long) 1000000 * clockSec;
    #endif

    #else
    /*
     * RTI Connext DDS Micro takes the timestamp by GetSystemTimeAsFileTime,
     * this function should have a resolution of 100 nanoseconds but
     * GetSystemTimeAsFileTime is a non-realtime time & busy loop (very fast)
     * in implementation. The system time is obtained from SharedUserData, which
     * is fill by system on every hardware clock interruption. MICRO-2099
     *
     * More info here:
     * https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
     *
     * In order to obtain enough precission for a latencyTest we are going to
     * use the native API QueryPerformanceCounter function measured in
     * microseconds as RTI Connext DDS Pro does.
     */
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart / (unsigned long long) (_frequency /1000000.0);

    #endif /* RTI_WIN32 */
  #else
    clock_gettime(CLOCK_MONOTONIC, &timeStruct);
    return (timeStruct.tv_sec * ONE_BILLION) + timeStruct.tv_nsec;
  #endif /* RTI_PERFTEST_NANO_CLOCK */
}

void PerftestClock::milliSleep(unsigned int millisec)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(millisec));
}

void PerftestClock::sleep(const struct DDS_Duration_t& sleep_period)
{
    NDDSUtility::sleep(sleep_period);
}

/********************************************************************/
/* Micro OnSpawed Method */

struct PerftestMicroThreadOnSpawnedMethod
{
    MicroThreadOnSpawnedMethod method;
    void *thread_param;

};

static RTI_BOOL perftestMicroThreadRoutine(struct OSAPI_ThreadInfo *thread_info)
{
    PerftestMicroThreadOnSpawnedMethod *data = (PerftestMicroThreadOnSpawnedMethod *)thread_info->user_data;

    data->method(data->thread_param);
    delete(data);

    thread_info->stop_thread = RTI_TRUE;
    return RTI_TRUE;
}

/********************************************************************/
/* Thread Related functions */

struct PerftestThread* PerftestThread_new(
        const char *name,
        int threadPriority,
        int threadOptions,
        MicroThreadOnSpawnedMethod method,
        void *threadParam)
{
    struct OSAPI_ThreadProperty prio = OSAPI_ThreadProperty_INITIALIZER;
    PerftestMicroThreadOnSpawnedMethod *data = new PerftestMicroThreadOnSpawnedMethod();
    data->method = method;
    data->thread_param = threadParam;

    struct OSAPI_Thread *thread = NULL;
    thread = OSAPI_Thread_create(
                name,
                &prio,
                perftestMicroThreadRoutine,
                data,
                NULL);
    if (!OSAPI_Thread_start(thread)) {
        return NULL;
    }
    return thread;
}

#endif //#ifndef RTI_USE_CPP_11_INFRASTRUCTURE

#endif // PERFTEST_CERT
