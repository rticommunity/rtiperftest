#ifndef __PERFTEST_CPP_H__
#define __PERFTEST_CPP_H__

/*
 * (c) 2005-2018 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "MessagingIF.h"

class perftest_cpp
{
  public:
    perftest_cpp();
    ~perftest_cpp();

    int Run(int argc, char *argv[]);
    bool ParseConfig(int argc, char *argv[]);

  private:
    int Publisher();
    int Subscriber();

    unsigned long  _DataLen;
    unsigned int  _BatchSize;
    int  _SamplesPerBatch;
    unsigned long long _NumIter;
    bool _IsPub;
    bool _isScan;
    std::vector<unsigned long> _scanDataLenSizes;
    bool _UseReadThread;
    unsigned long long _SpinLoopCount;
    unsigned long _SleepNanosec;
    int  _LatencyCount;
    int  _NumSubscribers;
    int  _NumPublishers;
    unsigned long _InstanceCount;
    IMessaging *_MessagingImpl;
    char **_MessagingArgv;
    int _MessagingArgc;
    bool _LatencyTest;
    bool _IsReliable;
    int _pubRate;
    bool _pubRateMethodSpin;
    bool _isKeyed;
    unsigned long _useUnbounded;
    unsigned int _executionTime;
    bool _displayWriterStats;
    bool _useCft;

    /* The following three members are used in a static callback
       and so they have to be static */
    static bool _testCompleted;
    static bool _testCompleted_scan;
    static void Timeout();
    static void Timeout_scan();

  public:
    static int  _SubID;
    static int  _PubID;
    static bool _PrintIntervals;
    static bool _showCpu;

    static const char *_LatencyTopicName;
    static const char *_ThroughputTopicName;
    static const char *_AnnouncementTopicName;

    // Number of bytes sent in messages besides user data
    static const int OVERHEAD_BYTES = 28;
    // Flag used to indicate message is used for initialization only
    static const int INITIALIZE_SIZE = 1234;
    // Flag used to indicate end of test
    static const int FINISHED_SIZE = 1235;
    // Flag used to data packet length is changing
    static const int LENGTH_CHANGED_SIZE = 1236;

};

#endif // __PERFTEST_CPP_H__
