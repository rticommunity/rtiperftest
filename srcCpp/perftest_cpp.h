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
#include <limits.h>

#ifdef RTI_WIN32
  #include <windows.h>
#else
  #ifndef RTI_VXWORKS
    #include <sys/time.h>
  #endif
  #include <sched.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <signal.h>
#endif
#include "ParameterManager.h"

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
    bool ParseConfig(int argc, char *argv[]);
    void PrintConfiguration();
    unsigned int GetSamplesPerBatch();

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

    static const DDS_ProductVersion_t GetDDSVersion();
    static const Perftest_ProductVersion_t GetPerftestVersion();
    static void PrintVersion();

    static void ThreadYield() {
  #ifdef RTI_WIN32
        Sleep(0);
  #else
        sched_yield();
  #endif
    }

  private:
    unsigned long  _DataLen;
    unsigned long long _NumIter;
    bool _isScan;
    std::vector<unsigned long> _scanDataLenSizes;
    unsigned long long _SpinLoopCount;
    unsigned long _SleepNanosec;
    int  _NumSubscribers;
    int  _NumPublishers;
    unsigned long _InstanceCount;
    IMessaging *_MessagingImpl;
    char **_MessagingArgv;
    int _MessagingArgc;
    bool _LatencyTest;
    int _pubRate;
    bool _pubRateMethodSpin;
    bool _displayWriterStats;
    bool _useCft;
    static const Perftest_ProductVersion_t _version;

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

    static struct RTIClock *_Clock;
    static struct RTINtpTime _ClockTime_aux;
    static RTI_UINT64 _Clock_sec;
    static RTI_UINT64 _Clock_usec;

  #ifdef RTI_WIN32
    static LARGE_INTEGER _ClockFrequency;
  #endif

    // Number of bytes sent in messages besides user data
    static const int OVERHEAD_BYTES = 28;
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
