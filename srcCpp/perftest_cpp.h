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

/*
 * This is needed by MilliSleep in VxWorks, since in some versions the usleep
 * function does not exist. In the rest of OS we won't make use of it.
 */
#if defined(RTI_VXWORKS)
  #include "ndds/ndds_cpp.h"
#endif
#include "perftest.h"
#include "ndds/ndds_cpp.h"
#include "MessagingIF.h"
#include "clock/clock_highResolution.h"
#include "osapi/osapi_ntptime.h"
#include "ParameterManager.h"

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
	static void MilliSleep(unsigned int millisec);
	static const DDS_ProductVersion_t GetDDSVersion();
	static const Perftest_ProductVersion_t GetPerftestVersion();
	static void PrintVersion();
	static void ThreadYield();
	static unsigned long long GetTimeUsec();

  #ifdef RTI_WIN32
    static VOID CALLBACK Timeout(PVOID lpParam, BOOLEAN timerOrWaitFired);
    static VOID CALLBACK Timeout_scan(PVOID lpParam, BOOLEAN timerOrWaitFired);
  #else
    static void Timeout(int sign);
    static void Timeout_scan(int sign);
  #endif

  private:
    int Publisher();
    int Subscriber();
    static void SetTimeout(
            unsigned int executionTimeInSeconds,
            bool isScan = false);

	// Private members
    ParameterManager _PM;
    unsigned long long _SpinLoopCount;
    unsigned long _SleepNanosec;
    IMessaging *_MessagingImpl;
    static const Perftest_ProductVersion_t _version;

    /*
     * The following three members are used in a static callback
     * and so they have to be static
     */
    static bool _testCompleted;
    static bool _testCompleted_scan;
  #ifdef RTI_WIN32
    static HANDLE _hTimerQueue;
    static HANDLE _hTimer;
  #endif

  public:
    static int  _SubID;
    static bool printIntervals;
    static bool showCpu;
    static struct RTIClock *_Clock;
    static struct RTINtpTime _ClockTime_aux;
    static RTI_UINT64 _Clock_sec;
    static RTI_UINT64 _Clock_usec;

  #ifdef RTI_WIN32
    static LARGE_INTEGER _ClockFrequency;
  #endif

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
