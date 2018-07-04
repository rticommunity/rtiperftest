/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __PERFTEST_READER_LISTENER_H__
#define __PERFTEST_READER_LISTENER_H__

#include "MessagingIF.h"
#include "CpuMonitor.h"
#include "perftest_cpp.h"
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

/*********************************************************
 * Listener for the Subscriber side
 *
 * Keeps stats on data received per second.
 * Returns a ping for latency packets
 */
class ThroughputListener : public IMessagingCB {
    /*TODO: check which member should be private */
    /*TODO: fix name convention */
public:
    unsigned long long packetsReceived;
    unsigned long long bytesReceived;
    unsigned long long missingPackets;
    int lastDataLength;
    bool changeSize;

    // store info for the last data set
    int intervalDataLength;
    unsigned long long intervalPacketsReceived;
    unsigned long long intervalBytesReceived;
    unsigned long long intervalMissingPackets;
    unsigned long long intervalTime, beginTime;
    float missingPacketsPercent;
    CpuMonitor cpu;

private:
    IMessagingReader *_reader;
    IMessagingWriter *_writer;
    unsigned long *_lastSeqNum;

    int _numPublishers;
    std::vector<int> _finishedPublishers;
    bool _useCft;

public:
    ThroughputListener(
            IMessagingWriter *writer,
            IMessagingReader *reader = NULL,
            bool UseCft = false,
            int numPublishers = 1);

    ~ThroughputListener();

    IMessagingReader *get_reader();

    void ProcessMessage(TestMessage &message);

    void print_summary(TestMessage &message);


};


/*********************************************************
 * Data listener for the Announcement
 *
 * Receives an announcement message from a Subscriber once
 * the subscriber has discovered every Publisher.
 */
class AnnouncementListener : public IMessagingCB {
public:
    std::vector<int> subscriberList;

    AnnouncementListener(){}

    void ProcessMessage(TestMessage &message);
};

/*********************************************************
 * Data listener for the Publisher side.
 *
 * Receives latency ping from Subscriber and does
 * round trip latency calculations
 */
class LatencyListener : public IMessagingCB {
private:
    unsigned long long _latencySum;
    unsigned long long _latencySumSquare;
    unsigned long long _count;
    unsigned long _latencyMin;
    unsigned long _latencyMax;
    int _lastDataLength;
    unsigned long *_latencyHistory;
    unsigned long _clockSkewCount;
    unsigned int _numLatency;
    IMessagingWriter *_writer;
    IMessagingReader *_reader;

public:
    CpuMonitor cpu;

public:
    LatencyListener(
            unsigned int numLatency,
            IMessagingReader *reader,
            IMessagingWriter *writer);

    ~LatencyListener();

    IMessagingReader *get_reader();

    void print_summary_latency();

    void ProcessMessage(TestMessage &message);
};


#endif