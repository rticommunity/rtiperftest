/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __PERFTEST_CPP_H__
#define __PERFTEST_CPP_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <limits.h>

#include "clock/clock_highResolution.h"
#include "osapi/osapi_ntptime.h"

#include <rti/config/Version.hpp>

#include "RTIDDSImpl.h"
#include "MessagingIF.h"
#include "perftest.hpp"
#include "ParameterManager.h"
#include "PerftestPrinter.h"
#include <chrono>

#ifdef RTI_WIN32
  #include <windows.h>
#elif defined RTI_INTIME
#else
  #ifndef RTI_VXWORKS
    #include <sys/time.h>
  #endif
  #include <sched.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <signal.h>
#endif

#include <rti/util/util.hpp>
#include "MessagingIF.h"
#include "ThreadPriorities.h"

#define PERFTEST_DISCOVERY_TIME_MSEC 1000   // 1 second
#define ONE_MILLION 1000000L     // 1 million == 1 second in us

struct Perftest_ProductVersion_t
{
    char major;
    char minor;
    char release;
    char revision;
};

class PerftestClock {
public:
    static unsigned long long getTime();
};

class perftest_cpp
{
  public:
    perftest_cpp();
    ~perftest_cpp();
    int Run(int argc, char *argv[]);
    bool validate_input();
    void print_configuration();
    unsigned int get_samples_per_batch();
    const ThreadPriorities get_thread_priorities();
    static void MilliSleep(unsigned int millisec);
    static const rti::core::ProductVersion GetDDSVersion();
    static const Perftest_ProductVersion_t get_perftest_version();
    static void print_version();
    static void ThreadYield();
    static unsigned long long GetTimeUsec();
    static void Timeout();

  private:
    struct ScheduleInfo {
        unsigned int timer;
        void (*handlerFunction)(void);
    };

    int RunPublisher();
    int RunSubscriber();
    static void *waitAndExecute(void *scheduleInfo);
    static RTIOsapiThread *SetTimeout(ScheduleInfo &info);

    // Private members
    ParameterManager _PM;
    PerftestPrinter *_printer;
    unsigned long long _SpinLoopCount;
    unsigned long _SleepNanosec;
    IMessaging *_MessagingImpl;
    static const Perftest_ProductVersion_t _version;

    ThreadPriorities _threadPriorities;

  public:

    /* The following three members are used in a static callback
       and so they have to be static */
    static bool _testCompleted;

    /*
     * Number of bytes sent in messages besides user data. This value is
     * calculated at run time.
     */
    static unsigned int OVERHEAD_BYTES;

    // Flag used to indicate message is used for initialization only
    static const int INITIALIZE_SIZE = 1234;
    // Flag used to indicate end of test
    static const int FINISHED_SIZE = 1235;

    /*
     * Value used to compare against to check if the latency_min has
     * been reset.
     */
    static const unsigned long LATENCY_RESET_VALUE = ULONG_MAX;

    int  subID;
    bool printIntervals;
    bool showCpu;

};

#endif // __PERFTEST_CPP_H__
