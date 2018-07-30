/*
 * (c) 2005-2018 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef INFRASTRUCTURE_PRO_H_
#define INFRASTRUCTURE_PRO_H_

#ifdef RTI_MICRO

#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_time.h"
#include "rti_me_cpp.hxx"
#include "PerftestTransport.h"

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

#define PerftestLogParamNotMicro(param) \
    fprintf(stderr, "The command-line parameter \"%s\" is not supported in RTI Perftest for Micro\n", param); \
    return false;
#define PerftestLogNotSupportedInMicro(message) \
    fprintf(stderr, "%s is not supported in RTI Perftest for Micro\n", message); \
    return false;


/* Perftest Clock Class */
class PerftestClock {

  private:
    OSAPI_NtpTime clockTimeAux;
    RTI_INT32 clockSec;
    RTI_UINT32 clockUsec;

  public:
    PerftestClock();
    ~PerftestClock();

    static PerftestClock &getInstance();
    unsigned long long getTimeUsec();
    static void milliSleep(unsigned int millisec);

};

bool PerftestConfigureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos);

/*
 * The following structures/classes are copied from RTI Connext DDS
 */

typedef struct RTINtpTime {
    /*e Seconds.**/
    RTI_INT32  sec;
    /*e fraction of a second in 1/2^32 form.*/
    RTI_UINT32 frac;
} RTINtpTime;

#define RTI_NTP_TIME_ZERO {0,0}

#define RTINtpTime_unpackToMicrosec(s, usec, time)     \
{                                                      \
    register RTI_UINT32 RTINtpTime_temp = (time).frac; \
    s    = (time).sec;                                 \
    usec = ((time).frac-                               \
        (RTINtpTime_temp>>5)-                          \
        (RTINtpTime_temp>>7)-                          \
        (RTINtpTime_temp>>8)-                          \
            (RTINtpTime_temp>>9)-                      \
        (RTINtpTime_temp>>10)-                         \
        (RTINtpTime_temp>>12)-                         \
            (RTINtpTime_temp>>13)-                     \
        (RTINtpTime_temp>>14) + (1<<11)) >> 12;        \
    if( ((usec) >= 1000000) && ((s)!=0x7FFFFFFF) ) {   \
        (usec) -= 1000000;                             \
        (s)++;                                         \
    }                                                  \
}

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

#define RTINtpTime_packFromFractionPrecise(time, \
                                           numerator, denominator_per_sec, \
                                           precisionBits) \
{ \
    register RTI_UINT32 RTINtpTime_current_bit = 0x80000000; \
    register RTI_UINT32 RTINtpTime_current_units = (denominator_per_sec)<<precisionBits; \
    register RTI_UINT32 RTINtpTime_precision_numerator = \
        ((numerator)%(denominator_per_sec))<< precisionBits; \
    (time).sec  = (numerator)/(denominator_per_sec); \
    (time).frac = 0; \
    while (RTINtpTime_current_units >>= 1) { \
        (time).frac += \
            (((RTINtpTime_precision_numerator) >= RTINtpTime_current_units) ? \
             (((RTINtpTime_precision_numerator) -= RTINtpTime_current_units), RTINtpTime_current_bit) : 0); \
        RTINtpTime_current_bit >>= 1; \
    } \
}

#define RTINtpTime_packFromMicrosec(time, s, usec) \
{ \
    register RTI_UINT32 RTINtpTime_temp = usec; \
    (time).sec  = s; \
    (time).frac = (RTINtpTime_temp<<12)+ \
		  ((RTINtpTime_temp*99)<<1)+ \
		  ((RTINtpTime_temp*15 + \
		  ((RTINtpTime_temp*61)>>7))>>4); \
}

class NDDSUtility {
  public:
    /*e \dref_Utility_sleep */
    static void sleep(const struct DDS_Duration_t& durationIn){
        /* following is a copy of OSAPI_Thread_sleep() in POSIX.
         * for other platforms we need to find out how to sleep
         * with a nanosecond precession
         */
    #if defined(RTI_LINUX) || defined(RTI_DARWIN)
        RTI_INT32 is;
        struct timespec remain,next;
        int rval;

        next.tv_sec = durationIn.sec;
        next.tv_nsec = durationIn.nanosec;

        #if defined(__APPLE__)
        do
        {
            rval = nanosleep(&next,&remain);
            if ((rval == -1) && (errno == EINTR))
            {
                next = remain;
            }
        } while ((rval == -1) && (errno == EINTR));
        #elif !defined(USE_TIMER_THREAD_SLEEP) && !defined(RTI_UCLINUX)
        do
        {
            rval = clock_nanosleep(CLOCK_REALTIME,0,&next,&remain);
            if (rval == EINTR)
            {
                next = remain;
            }
        } while (rval == EINTR);
        #else
        do
        {
            rval = nanosleep(&next,&remain);
            if ((rval == -1) && (errno == EINTR))
            {
                next = remain;
            }
        } while ((rval == -1) && (errno == EINTR));
        #endif
    #else
        /* for other platforms sleep only if time to sleep is greater than 0 ms */
        RTI_UINT32 ms = durationIn.sec * 1000 + durationIn.nanosec / 1000000;
        if (ms > 0)
            OSAPI_Thread_sleep(ms);
    #endif
    };

    /*e \dref_Utility_spin */
    static void spin(DDS_UnsignedLongLong spinCount){
        NDDS_Utility_spin(spinCount);
    };


    static DDS_UnsignedLongLong get_spin_per_microsecond() {

        /* Same default values used by DDS */
        unsigned int spinCount = 20000;
        unsigned int rti_clock_calculation_loop_count_max = 100;

        unsigned long long init, fin, usec, spinsPerUsec;

        PerftestClock clock = PerftestClock::getInstance();
        init = clock.getTimeUsec();

        for (int i = 0; i < rti_clock_calculation_loop_count_max; ++i) {
            NDDS_Utility_spin(spinCount);
        }
        fin = clock.getTimeUsec();

        usec = fin - init;

        spinsPerUsec = 1.0 / ((float)usec
                / (float)(rti_clock_calculation_loop_count_max * spinCount));

        /*
         * The value may be 0 due to lack of resolution or non
         * monotonic clocks.
         */
        if (spinsPerUsec < 1) {
            spinsPerUsec = 1; // To avoid error
        }

        return spinsPerUsec;
    }

};

typedef void *(*MicroThreadOnSpawnedMethod)(void*);

bool PerftestCreateThread(
        const char *name,
        MicroThreadOnSpawnedMethod method,
        void *threadParam);

void PerftestConfigureVerbosity(int verbosityLevel);

#endif // RTI_MICRO
#endif /* INFRASTRUCTURE_PRO_H_ */
