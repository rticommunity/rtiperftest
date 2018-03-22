#ifndef __PERFTEST_CPP_H__
#define __PERFTEST_CPP_H__

/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string>
#include <vector>
// STL needed for sorting
#include <algorithm>

#ifdef RTI_WIN32
  #include <windows.h>
#else
  #include <sys/time.h>
  #include <sched.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <signal.h>
#endif

/*
 * This is needed by MilliSleep in VxWorks, since in some versions the usleep
 * function does not exist. In the rest of OS we won't make use of it.
 */
#if defined(RTI_VXWORKS)
  #include "ndds/ndds_cpp.h"
#endif

#include "MessagingIF.h"
#include "clock/clock_highResolution.h"
#include "osapi/osapi_ntptime.h"

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

  public:
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

    static void ThreadYield() {
  #ifdef RTI_WIN32
        Sleep(0);
  #else
        sched_yield();
  #endif
    }

  private:
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

  private:
    static void SetTimeout(unsigned int executionTimeInSeconds, bool _isScan = false);

    /* The following three members are used in a static callback
       and so they have to be static */
    static bool _testCompleted;
    static bool _testCompleted_scan;
  #ifdef RTI_WIN32
    static HANDLE _hTimerQueue;
    static HANDLE _hTimer;
  #endif

  public:
    static int  _SubID;
    static int  _PubID;
    static bool _PrintIntervals;
    static bool _showCpu;

    static struct RTIClock *_Clock;
    static struct RTINtpTime _ClockTime_aux;
    static RTI_UINT64 _Clock_sec;
    static RTI_UINT64 _Clock_usec;

    static const char *_LatencyTopicName;
    static const char *_ThroughputTopicName;
    static const char *_AnnouncementTopicName;

  #ifdef RTI_WIN32
    static LARGE_INTEGER _ClockFrequency;
  #endif

    // Number of bytes sent in messages besides user data
  #ifdef RTI_CUSTOM_TYPE
    static const int OVERHEAD_BYTES = 28 + 4; // +4 for the size_customType
  #else
    static const int OVERHEAD_BYTES = 28;
  #endif
    // Flag used to indicate message is used for initialization only
    static const int INITIALIZE_SIZE = 1234;
    // Flag used to indicate end of test
    static const int FINISHED_SIZE = 1235;
    // Flag used to data packet length is changing
    static const int LENGTH_CHANGED_SIZE = 1236;

   public:
    static unsigned long long GetTimeUsec();

  #ifdef RTI_WIN32
    static VOID CALLBACK Timeout(PVOID lpParam, BOOLEAN timerOrWaitFired);
    static VOID CALLBACK Timeout_scan(PVOID lpParam, BOOLEAN timerOrWaitFired);
  #else
    static void Timeout(int sign);
    static void Timeout_scan(int sign);
  #endif

};

#endif // __PERFTEST_CPP_H__
