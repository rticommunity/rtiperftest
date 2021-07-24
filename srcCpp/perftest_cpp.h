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
#include "FileDataLoader.h"
#include "PerftestPrinter.h"

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
    void print_configuration();
    unsigned int get_samples_per_batch();
    const ThreadPriorities get_thread_priorities();
    void calculate_publication_rate();
    static const Perftest_ProductVersion_t get_perftest_version();
    static void print_version();

  private:
    int Publisher();
    int Subscriber();
    template <class ListenerType>
    bool finalize_read_thread(PerftestThread *thread, ListenerType *listener);

    // Private members
    ParameterManager _PM;
    PerftestPrinter *_printer;
    unsigned long long _SpinLoopCount;
    unsigned long _SleepNanosec;
    IMessaging *_MessagingImpl;
    static const Perftest_ProductVersion_t _version;
    FileDataLoader _fileDataLoader;

    // Priorities for the threads used by perftest and domain participant
    ThreadPriorities _threadPriorities;


    static void Timeout();
    static void Timeout_scan();

  public:
    /*
     * The following members are used in a static callback
     * and so they have to be static
     */
    static bool _testCompleted;
    static bool _testCompleted_scan;

    int  subID;
    bool printIntervals;
    bool showCpu;

    // Number of bytes sent in messages besides user data
    static unsigned int OVERHEAD_BYTES;
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
