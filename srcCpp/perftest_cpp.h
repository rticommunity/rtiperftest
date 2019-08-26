#ifndef __PERFTEST_CPP_H__
#define __PERFTEST_CPP_H__

/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <limits.h>

#include "MessagingIF.h"
#include "ThreadPriorities.h"
#include "ParameterManager.h"
#include "Infrastructure_common.h"

struct Perftest_ProductVersion_t
{
    char major;
    char minor;
    char release;
    char revision;
};

class perftest_cpp
{
  public:
    perftest_cpp();
    ~perftest_cpp();
    int Run(int argc, char *argv[]);
    bool validate_input();
    void PrintConfiguration();
    unsigned int GetSamplesPerBatch();
    const ThreadPriorities get_thread_priorities();
    static const Perftest_ProductVersion_t GetPerftestVersion();
    static void PrintVersion();

  private:
    int Publisher();
    int Subscriber();
    template <class ListenerType>
    bool finalize_read_thread(PerftestThread *thread, ListenerType *listener);

    // Private members
    ParameterManager _PM;
    unsigned long long _SpinLoopCount;
    unsigned long _SleepNanosec;
    IMessaging *_MessagingImpl;
    static const Perftest_ProductVersion_t _version;

    // Priorities for the threads used by perftest and domain participant
    ThreadPriorities _threadPriorities;

    /*
     * The following members are used in a static callback
     * and so they have to be static
     */
    static bool _testCompleted;
    static bool _testCompleted_scan;
    static void Timeout();
    static void Timeout_scan();

  public:
    int  subID;
    bool printIntervals;
    bool showCpu;

    // Number of bytes sent in messages besides user data
  #ifdef RTI_CUSTOM_TYPE
    static const int OVERHEAD_BYTES = 28 + 4; // 4 for custom_type_size
  #else
    static const int OVERHEAD_BYTES = 28;
  #endif
    // Flag used to indicate message is used for initialization only
    static const int INITIALIZE_SIZE = 1234;
    // Flag used to indicate end of test
    static const int FINISHED_SIZE = 1235;
    // Flag used to data packet length is changing
    static const int LENGTH_CHANGED_SIZE = 1236;

    /*
     * Value used to compare against to check if the latency_min has
     * been reset.
     */
    static const unsigned long LATENCY_RESET_VALUE = ULONG_MAX;
};

#endif // __PERFTEST_CPP_H__
