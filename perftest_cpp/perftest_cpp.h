#ifndef __PERFTEST_CPP_H__
#define __PERFTEST_CPP_H__

/* $Id: perftest_cpp.h,v 1.12 2015/07/22 22:17:26 jmorales Exp $

 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
 Permission to modify and use for internal purposes granted.   	
 This software is provided "as is", without warranty, express or implied.

 Modification History
 --------------------
 5.2.0,22jul15,jm  PERFTEST-90 The getTime function uses now our high precision clock.
 5.2.0,27apr14,jm  PERFTEST-86 Removing .ini support. Fixing warnings.
 5.1.0,20aug14,jmc PERFTEST-61 Added -executionTime command line option
 5.1.0,11aug14,jm  PERFTEST-57 Added -keyed command line option.
 5.1.0,15jul14,jmc Modifying _SpinLoopCount to be unsigned long long
 5.1.0,15jul14,jmc PERFTEST-51 Added _pubRate
 1.0a,13jul10,jsr Added _isReliable field for the latnecy option and
                  bestEffort mode
 1.0a,13may09,fcs Removed InitDDS & Added StringDup
 1.0a,29may09,jsr Added detection of wrong command line parameter
 1.0a,14aug08,ch  changed key to be 4 bytes long -> overhead change
 1.0a,08may08,ch  Added keyed type support (overhead changed) 
 1.0a,15apr08,fcs Added _BatchSize, _SamplesPerBatch, _NumPublishers 
                  and _PubID
 1.0a,19mar08,hhw Created.
===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

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
  #ifdef RTI_WIN32
        Sleep(millisec);
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
    int  _DataLen;
    int  _BatchSize;
    int  _maxBinDataSize;
    int  _SamplesPerBatch;
    unsigned long long _NumIter;
    bool _IsPub;
    bool _IsScan;
    bool _UseReadThread;
    unsigned long long _SpinLoopCount;
    int  _SleepMillisec;
    int  _LatencyCount;
    int  _NumSubscribers;
    int  _NumPublishers;
    int _InstanceCount;
    IMessaging *_MessagingImpl;
    char **_MessagingArgv;
    int _MessagingArgc;
    bool _LatencyTest;
    bool _IsReliable;
    int _pubRate;
    bool _isKeyed;
    unsigned int _executionTime;

  private:
    static char * StringDup(const char * str);
    static void SetTimeout(unsigned int executionTimeInSeconds);

    /* The following three members are used in a static callback
       and so they have to be static */
    static bool _testCompleted;
  #ifdef RTI_WIN32
    static HANDLE _hTimerQueue;
    static HANDLE _hTimer;
  #endif

  public:
    static int  _SubID;
    static int  _PubID;
    static bool _PrintIntervals;
    static bool _IsDebug;
    static struct RTIClock *clock;
    static const char *_LatencyTopicName;
    static const char *_ThroughputTopicName;
    static const char *_AnnouncementTopicName;

  #ifdef RTI_WIN32
    static LARGE_INTEGER _ClockFrequency;
  #endif
    
    // Number of bytes sent in messages besides user data
    static const int OVERHEAD_BYTES = 28;
    
    // When running a scan, this determines the number of
    // latency pings that will be sent before increasing the 
    // data size
    static const int NUM_LATENCY_PINGS_PER_DATA_SIZE = 1000;
    
    // Flag used to indicate message is used for initialization only
    static const int INITIALIZE_SIZE = 1234;
    // Flag used to indicate end of test
    static const int FINISHED_SIZE = 1235;
    // Flag used to data packet length is changing
    static const int LENGTH_CHANGED_SIZE = 1236;

   public:
    static unsigned long long GetTimeUsec(struct RTINtpTime *now);

  #ifdef RTI_WIN32
    static VOID CALLBACK Timeout(PVOID lpParam, BOOLEAN timerOrWaitFired);
  #else
    static void Timeout(int sign);
  #endif

};

#endif // __PERFTEST_CPP_H__
