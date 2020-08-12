/*
 * (c) 2005-2020 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_CYCLONEDDS_H_
#define INFRASTRUCTURE_CYCLONEDDS_H_

/*
 * For the time being, the implementation of CycloneDDS will rely on the
 * infrastructure for Micro, still, we will leave this file in place so we can
 * replace it at some point.
 */

#ifdef PERFTEST_CYCLONEDDS

#include "dds/version.h"
#include "dds/ddsrt/environ.h"

//TODO: Remove dependencies with Micro
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_thread.h"
#include "PerftestTransport.h"

#ifdef RTI_WIN32
  #include "windows.h"
#endif

#include <sstream>

// This should be temporal.
static const char * const THROUGHPUT_TOPIC_NAME= "Throughput";
static const char * const LATENCY_TOPIC_NAME= "Latency";
static const char * const ANNOUNCEMENT_TOPIC_NAME= "Announcement";

/********************************************************************/
/*
 * In order to unify the implementations for Micro and Pro, we wrap the
 * semaphores to a common PerftestSemaphore implementation.
 */

#define PerftestSemaphore OSAPI_Semaphore_T
#define PerftestSemaphore_new OSAPI_Semaphore_new
#define PerftestSemaphore_delete OSAPI_Semaphore_delete
#define PerftestSemaphore_give OSAPI_Semaphore_give
#define PERFTEST_SEMAPHORE_TIMEOUT_INFINITE OSAPI_SEMAPHORE_TIMEOUT_INFINITE

inline RTI_BOOL PerftestSemaphore_take(PerftestSemaphore *sem, int timeout)
{
    return OSAPI_Semaphore_take(sem, timeout, NULL);
}

#define PERFTEST_DISCOVERY_TIME_MSEC 1000 // 1 second
#define ONE_BILLION  1000000000L // 1 billion (US) == 1 second in ns
#define ONE_MILLION  1000000L // 1 million == 1 second in us

#define PerftestMutex OSAPI_Mutex_T
#define PerftestMutex_new OSAPI_Mutex_new
#define PerftestMutex_delete OSAPI_Mutex_delete
#define PerftestMutex_give OSAPI_Mutex_give
#define PerftestMutex_take OSAPI_Mutex_take

/* Perftest Clock Class */
class PerftestClock {

  private:
    struct timespec timeStruct;

  public:
    PerftestClock();
    ~PerftestClock();

    static PerftestClock &getInstance();
    unsigned long long getTime();
    static void milliSleep(unsigned int millisec);
    static void sleep(const struct DDS_Duration_t& sleep_period);

};

//TODO: Change all uses to the specific DDS Implementation.
const std::string GetDDSVersionString();

/********************************************************************/
/* THREADS */

#define PerftestThread OSAPI_Thread

#define Perftest_THREAD_PRIORITY_DEFAULT 0
#define Perftest_THREAD_SETTINGS_REALTIME_PRIORITY 0
#define Perftest_THREAD_SETTINGS_PRIORITY_ENFORCE 0
#define Perftest_THREAD_OPTION_DEFAULT 0

typedef void *(*MicroThreadOnSpawnedMethod)(void*);

struct PerftestThread* PerftestThread_new(
        const char *name,
        int threadPriority,
        int threadOptions,
        MicroThreadOnSpawnedMethod method,
        void *threadParam);

void PerftestThread_delete(struct PerftestThread* thread);

#define NDDS_Utility_spin(spinCount)                    \
{                                                       \
    RTI_UINT64 spin;                                    \
    RTI_UINT64 ad, bd, cd;                              \
    volatile RTI_UINT64 * a, * b, * c;                  \
    a = &ad;                                            \
    b = &bd;                                            \
    c = &cd;                                            \
    for (spin = 0; spin < (spinCount); ++spin) {        \
        *a = 3;                                         \
        *b = 1;                                         \
        *c = (*a/(*b))*spin;                            \
    }                                                   \
}

struct DDS_Duration_t
{
    long sec;
    unsigned long nanosec;
};


class NDDSUtility
{
  public:
//     /*e \dref_Utility_sleep */
    static void sleep(const struct DDS_Duration_t& durationIn){
//         /* following is a copy of OSAPI_Thread_sleep() in POSIX.
//          * for other platforms we need to find out how to sleep
//          * with a nanosecond precession
//          */
//     #if defined(RTI_LINUX) || defined(RTI_DARWIN)
//         RTI_INT32 is;
//         struct timespec remain,next;
//         int rval;

//         next.tv_sec = durationIn.sec;
//         next.tv_nsec = durationIn.nanosec;

//         #if defined(__APPLE__)
//         do
//         {
//             rval = nanosleep(&next,&remain);
//             if ((rval == -1) && (errno == EINTR))
//             {
//                 next = remain;
//             }
//         } while ((rval == -1) && (errno == EINTR));
//         #elif !defined(USE_TIMER_THREAD_SLEEP) && !defined(RTI_UCLINUX)
//         do
//         {
//             rval = clock_nanosleep(CLOCK_REALTIME,0,&next,&remain);
//             if (rval == EINTR)
//             {
//                 next = remain;
//             }
//         } while (rval == EINTR);
//         #else
//         do
//         {
//             rval = nanosleep(&next,&remain);
//             if ((rval == -1) && (errno == EINTR))
//             {
//                 next = remain;
//             }
//         } while ((rval == -1) && (errno == EINTR));
//         #endif
//     #else
//         /* for other platforms sleep only if time to sleep is greater than 0 ms */
//         RTI_UINT32 ms = durationIn.sec * 1000 + durationIn.nanosec / 1000000;
//         if (ms > 0)
//             OSAPI_Thread_sleep(ms);
//     #endif
    };

    /*e \dref_Utility_spin */
    static void spin(unsigned long long spinCount){
        NDDS_Utility_spin(spinCount);
    };


    static unsigned long long
    get_spin_per_microsecond(unsigned int precision = 100)
    {
        /* Same default values used by DDS */
        unsigned int spinCount = 200000;
        unsigned long long clockCalculationLoopCountMax = 100;

        unsigned long long usec = 0;
        unsigned long long iterations = 0;

        PerftestClock clock = PerftestClock::getInstance();

        do{
            usec = clock.getTime(); // Initial time
            NDDS_Utility_spin(spinCount * iterations);
            usec = clock.getTime() - usec; // Final time
            iterations++;
            /*
             * If the the clock have a low precision, increase spinCount
             * until we measure some us or reach a maximun count loop
             */
        } while (usec < precision && iterations < clockCalculationLoopCountMax);

        /*
         * The measure time can be zero due to lack of resolution or non
         * monotonic clocks and a really fast machine.
         * We may end on a exception condition, unable to calculate the
         * spins per micro-seconds.
         */
        if (usec == 0) {
            fprintf(stderr,
                    "Unable to calculate the number of spins per"
                    "micro-seconds\n");
            return 0;
        }
        return (unsigned long long) (iterations * spinCount) / usec;
    }
};

#endif // PERFTEST_CYCLONEDDS
#endif /* INFRASTRUCTURE_CYCLONEDDS_H_ */
